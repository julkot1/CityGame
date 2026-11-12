#include "simulation/SimulationThread.h"
#include "world/WorldStorage.h"
#include "world/RandomWorldGenerator.h"
#include <chrono>

namespace Magnum {

namespace {

Vector3 rotateYQuarterTurns(const Vector3& v, UnsignedInt rot) {
    switch(rot & 3u) {
        case 1u: return Vector3{ v.z(), v.y(), -v.x()};
        case 2u: return Vector3{-v.x(), v.y(), -v.z()};
        case 3u: return Vector3{-v.z(), v.y(),  v.x()};
        default: return v;
    }
}

}

SimulationThread::SimulationThread(World& world):
    _world{world}
{
    // Initialize baseline snapshot
    _latestSnapshot.timeOfDay = _timeManager.timeOfDay();
    _latestSnapshot.hour = _timeManager.hour();
    _latestSnapshot.minute = _timeManager.minute();
    _latestSnapshot.formattedTime = _timeManager.formattedTime();
    _latestSnapshot.isPaused = _timeManager.isPaused();
    _latestSnapshot.timeSpeed = _timeManager.timeSpeed();
    _latestSnapshot.lightDirection = _timeManager.lightDirection();
    _latestSnapshot.lightColor = _timeManager.lightColor();
    _latestSnapshot.ambientColor = _timeManager.ambientColor();
    _latestSnapshot.skyColor = _timeManager.skyColor();
    _latestSnapshot.isNight = _timeManager.isNight();
}

SimulationThread::~SimulationThread() {
    stop();
}

void SimulationThread::start() {
    if(_running.exchange(true)) return;
    updateChimneyPositions();
    _thread = std::thread(&SimulationThread::run, this);
}

void SimulationThread::stop() {
    if(!_running.exchange(false)) return;
    if(_thread.joinable()) {
        _thread.join();
    }
}

void SimulationThread::postCommand(SimulationCommand cmd) {
    _commandQueue.push(std::move(cmd));
}

void SimulationThread::drainMeshUploads(std::vector<ChunkMeshData>& outMeshes) {
    _meshUploadQueue.drain(outMeshes);
}

TimeLightingSnapshot SimulationThread::getLightingSnapshot() const {
    std::lock_guard<std::mutex> lock(_snapshotMutex);
    return _latestSnapshot;
}

std::vector<Vector3> SimulationThread::getChimneyPositions() const {
    std::lock_guard<std::mutex> lock(_chimneyMutex);
    return _chimneyPositions;
}

void SimulationThread::run() {
    using clock = std::chrono::steady_clock;
    constexpr auto targetTickDuration = std::chrono::microseconds(16666); // ~60 Hz
    auto nextTick = clock::now();

    UnsignedInt tickCount = 0;
    Float timeAccumulator = 0.0f;
    Float tickDurationAccumulator = 0.0f;

    while(_running.load(std::memory_order_relaxed)) {
        auto now = clock::now();
        if(now < nextTick) {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            continue;
        }

        nextTick += targetTickDuration;
        if(now > nextTick + targetTickDuration * 4) {
            nextTick = now; // Prevent catch-up spiral if thread was stalled
        }

        auto start = clock::now();
        tick(0.016666f);
        auto end = clock::now();

        Float tickDurationMs = std::chrono::duration<Float, std::milli>(end - start).count();
        tickCount++;
        timeAccumulator += 0.016666f;
        tickDurationAccumulator += tickDurationMs;

        if(timeAccumulator >= 1.0f) {
            _simTPS.store(Float(tickCount) / timeAccumulator, std::memory_order_relaxed);
            _simTickDurationMs.store(tickDurationAccumulator / Float(tickCount), std::memory_order_relaxed);
            tickCount = 0;
            timeAccumulator = 0.0f;
            tickDurationAccumulator = 0.0f;
        }
    }
}

void SimulationThread::tick(Float dt) {
    // 1. Drain and execute pending player commands
    std::vector<SimulationCommand> commands;
    _commandQueue.drain(commands);

    for(const auto& cmd : commands) {
        switch(cmd.type) {
            case SimulationCommandType::PlaceStructure: {
                std::unique_lock<std::shared_mutex> lock(_worldMutex);
                World::RaycastHit hit;
                hit.hit = true;
                hit.worldTileX = cmd.worldX;
                hit.worldTileZ = cmd.worldZ;
                hit.chunkCoord = World::worldToChunkCoord(cmd.worldX, cmd.worldZ);
                hit.localTileCoord = World::worldToLocalTileCoord(cmd.worldX, cmd.worldZ);

                _buildSystem.setActiveTool(cmd.tool);
                _buildSystem.setBuildingOrientation(cmd.orientation);

                std::vector<ChunkMeshData> dirtyMeshes;
                _buildSystem.executeAction(_world, hit, &dirtyMeshes);
                _meshUploadQueue.pushBatch(std::move(dirtyMeshes));
                updateChimneyPositions();
                break;
            }
            case SimulationCommandType::Demolish: {
                std::unique_lock<std::shared_mutex> lock(_worldMutex);
                World::RaycastHit hit;
                hit.hit = true;
                hit.worldTileX = cmd.worldX;
                hit.worldTileZ = cmd.worldZ;
                hit.chunkCoord = World::worldToChunkCoord(cmd.worldX, cmd.worldZ);
                hit.localTileCoord = World::worldToLocalTileCoord(cmd.worldX, cmd.worldZ);

                _buildSystem.setActiveTool(BuildTool::Demolish);

                std::vector<ChunkMeshData> dirtyMeshes;
                _buildSystem.executeAction(_world, hit, &dirtyMeshes);
                _meshUploadQueue.pushBatch(std::move(dirtyMeshes));
                updateChimneyPositions();
                break;
            }
            case SimulationCommandType::TimeTogglePause: {
                _timeManager.togglePaused();
                break;
            }
            case SimulationCommandType::TimeSetSpeed: {
                _timeManager.setTimeSpeed(cmd.timeValue);
                break;
            }
            case SimulationCommandType::TimeSetTimeOfDay: {
                _timeManager.setTimeOfDay(cmd.timeValue);
                break;
            }
            case SimulationCommandType::CreateNewWorld: {
                std::unique_lock<std::shared_mutex> lock(_worldMutex);
                auto randGen = dynamic_cast<RandomWorldGenerator*>(_world.generator());
                if(randGen) {
                    randGen->setArchetype(static_cast<WorldArchetype>(cmd.worldParams.archetype));
                    randGen->setFlatness(cmd.worldParams.flatness);
                    randGen->setMountainScale(cmd.worldParams.mountainScale);
                    randGen->setRiverIntensity(cmd.worldParams.riverIntensity);
                    randGen->setForestDensity(cmd.worldParams.forestDensity);
                    randGen->setResourceDensity(cmd.worldParams.resourceDensity);
                    randGen->setSeaLevel(cmd.worldParams.seaLevel);
                    randGen->setSeed(cmd.worldParams.seed);
                }

                _world.init(cmd.worldParams.worldWidthChunks, cmd.worldParams.worldHeightChunks);
                WorldStorage::saveToFile(_world, "world.save");

                std::vector<ChunkMeshData> allMeshes = _world.buildAllCpuMeshes();
                _meshUploadQueue.pushBatch(std::move(allMeshes));
                updateChimneyPositions();
                break;
            }
            case SimulationCommandType::SaveWorld: {
                std::shared_lock<std::shared_mutex> lock(_worldMutex);
                WorldStorage::saveToFile(_world, cmd.filename.empty() ? "world.save" : cmd.filename);
                break;
            }
        }
    }

    // 2. Advance time progression and city simulation
    _timeManager.update(dt);

    // 3. Publish thread-safe lighting and celestial snapshot for the renderer
    TimeLightingSnapshot snap;
    snap.timeOfDay = _timeManager.timeOfDay();
    snap.hour = _timeManager.hour();
    snap.minute = _timeManager.minute();
    snap.formattedTime = _timeManager.formattedTime();
    snap.isPaused = _timeManager.isPaused();
    snap.timeSpeed = _timeManager.timeSpeed();
    snap.lightDirection = _timeManager.lightDirection();
    snap.lightColor = _timeManager.lightColor();
    snap.ambientColor = _timeManager.ambientColor();
    snap.skyColor = _timeManager.skyColor();
    snap.isNight = _timeManager.isNight();

    {
        std::lock_guard<std::mutex> lock(_snapshotMutex);
        _latestSnapshot = std::move(snap);
    }
}

void SimulationThread::updateChimneyPositions() {
    std::vector<Vector3> chimneys;
    const Float tileSpacing = _world.tileSpacing();

    for(const auto& pair : _world.chunks()) {
        const Chunk& chunk = pair.second;
        const Vector3 chunkOrigin = chunk.worldPosition(tileSpacing);

        for(const Entity& entity : chunk.entities()) {
            Vector3 localChimney;
            bool hasChimney = false;

            if(entity.type == EntityType::House2x2) {
                localChimney = Vector3{0.55f, 3.12f, -0.27f};
                hasChimney = true;
            } else if(entity.type == EntityType::HouseStandalone2x2) {
                localChimney = Vector3{0.57f, 2.42f, -0.22f};
                hasChimney = true;
            } else if(entity.type == EntityType::CityCenter2x3) {
                localChimney = Vector3{0.56f, 2.70f, -1.22f};
                hasChimney = true;
            }

            if(hasChimney) {
                Vector3 chimneyPos = chunkOrigin + entity.position + rotateYQuarterTurns(localChimney, entity.data & 3u);
                chimneys.push_back(chimneyPos);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(_chimneyMutex);
        _chimneyPositions = std::move(chimneys);
    }
}

}
