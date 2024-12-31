<template>
    <main>
        <h2 class="mt-12 text-center text-white font-semibold text-3xl">Screen Sentry</h2>

        <p class="mt-12 text-center text-white">
            Status: <span class="text-red-400 font-semibold">{{ signalStatus }}</span>
        </p>

        <p class="mt-2 text-center text-white">
            Connected: <span class="text-violet-400 font-semibold">{{ isConnected ? 'TRUE' : 'FALSE' }}</span>
        </p>

        <div class="flex justify-center">
            <button
                @click="toggleSignal(SignalStatus.OFF)"
                :disabled="isLoading || signalStatus === SignalStatus.OFF"
                class="bg-zinc-700 px-5 py-2 rounded-md mt-12 font-semibold text-white mr-4"
            >
                Turn Signal Off
            </button>

            <button
                @click="toggleSignal(SignalStatus.ON)"
                :disabled="isLoading || signalStatus === SignalStatus.ON"
                class="bg-violet-500 px-5 py-2 rounded-md mt-12 font-semibold text-white"
            >
                Turn Signal On
            </button>
        </div>

        <div class="flex justify-center mt-6">
            <button @click="togglePolling" class="text-white text-center">
                {{ isPolling ? 'Stop' : 'Start' }} Polling
            </button>
        </div>
    </main>
</template>

<script setup lang="ts">
    import { ref, onMounted, onBeforeUnmount } from 'vue';

    enum SignalStatus {
        ON = 'ON',
        OFF = 'OFF'
    }

    const isLoading = ref<boolean>(true);
    const signalStatus = ref<SignalStatus>(SignalStatus.OFF);
    const isConnected = ref<boolean>(false);
    const isPolling = ref<boolean>(true);
    let pollingInterval: number | undefined;
    const backendURL = 'http://localhost:8000';

    function togglePolling() {
        if (isPolling.value) {
            stopPolling();
        } else {
            startPolling();
        }
    }

    function startPolling() {
        isPolling.value = true;
        isLoading.value = true;
        fetchSignalStatus();
        fetchConnectionStatus();

        pollingInterval = setInterval(() => {
            fetchSignalStatus();
            fetchConnectionStatus();
        }, 5000);
    }

    function stopPolling() {
        isPolling.value = false;
        if (pollingInterval) {
            clearInterval(pollingInterval);
        }
    }

    async function fetchSignalStatus() {
        try {
            const response = await fetch(`${backendURL}/api/signal-status`);

            if (!response.ok) {
                throw new Error('Failed to fetch signal status');
            }

            const data = await response.json();

            signalStatus.value = data.signalStatus;
        } catch (error) {
            console.error('Error fetching signal status: ', error);
        } finally {
            isLoading.value = false;
        }
    }

    async function fetchConnectionStatus() {
        try {
            const response = await fetch(`${backendURL}/api/connection-status`);

            if (!response.ok) {
                throw new Error('Failed to fetch connection status');
            }

            const data = await response.json();

            isConnected.value = data.isConnected;
        } catch (error) {
            console.error('Error fetching connection status: ', error);
        }
    }

    async function toggleSignal(status: SignalStatus) {
        isLoading.value = true;

        try {
            const response = await fetch(`${backendURL}/api/toggle-signal`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ status })
            });

            if (!response.ok) {
                throw new Error('Failed to toggle signal');
            }

            const data = await response.json();

            if (data.success) {
                signalStatus.value = data.status;
            } else {
                throw new Error(data.message || 'Failed to toggle signal');
            }
        } catch (error) {
            console.error(`Error toggling signal to ${status}: `, error);
        } finally {
            isLoading.value = false;
        }
    }

    onMounted(() => {
        startPolling();
    });

    onBeforeUnmount(() => {
        stopPolling();
    });
</script>
