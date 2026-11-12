#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <memory>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>
#include "world/World.h"
#include "world/TimeManager.h"
#include "world/ChunkMeshData.h"
#include "building/BuildSystem.h"
#include "threading/ConcurrentQueue.h"
#include "threading/SimulationCommand.h"
#include "threading/TimeLightingSnapshot.h"

namespace Magnum {

class SimulationThread {
    public:
        explicit SimulationThread(World& world);
        ~SimulationThread();

        void start();
        void stop();
        bool isRunning() const { return _running.load(); }

        // Thread-safe command queue for player actions (Build, Demolish, Time controls, etc.)
        void postCommand(SimulationCommand cmd);

        // Render thread drains finished CPU chunk meshes to upload to GPU
        void drainMeshUploads(std::vector<ChunkMeshData>& outMeshes);

        // Render thread reads lighting & sky snapshot without blocking simulation
        TimeLightingSnapshot getLightingSnapshot() const;

        // Render thread reads chimney positions for particle smoke rendering
        std::vector<Vector3> getChimneyPositions() const;

        // Shared mutex for non-blocking concurrent tile raycasting/inspection reads
        std::shared_mutex& worldMutex() { return _worldMutex; }

        // Simulation performance metrics
        Float tps() const { return _simTPS.load(); }
        Float tickDurationMs() const { return _simTickDurationMs.load(); }

        // Authoritative systems
        TimeManager& timeManager() { return _timeManager; }
        const TimeManager& timeManager() const { return _timeManager; }

        BuildSystem& buildSystem() { return _buildSystem; }
        const BuildSystem& buildSystem() const { return _buildSystem; }

    private:
        void run();
        void tick(Float dt);
        void updateChimneyPositions();

        World& _world;
        mutable std::shared_mutex _worldMutex;

        TimeManager _timeManager;
        BuildSystem _buildSystem;

        ConcurrentQueue<SimulationCommand> _commandQueue;
        ConcurrentQueue<ChunkMeshData> _meshUploadQueue;

        mutable std::mutex _snapshotMutex;
        TimeLightingSnapshot _latestSnapshot;

        mutable std::mutex _chimneyMutex;
        std::vector<Vector3> _chimneyPositions;

        std::atomic<bool> _running{false};
        std::thread _thread;

        std::atomic<Float> _simTPS{60.0f};
        std::atomic<Float> _simTickDurationMs{0.0f};
};

}
