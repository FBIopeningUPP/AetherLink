export function updateDashboard(data) {
    // 1. Existing Metrics
    document.getElementById('pack-voltage').textContent = (data.getUint32(32, true) / 1000).toFixed(2);
    document.getElementById('soc').textContent = data.getUint8(38);
    document.getElementById('cell-delta').textContent = data.getUint16(36, true);
    
    for(let i = 0; i < 12; i++) {
        const val = (data.getUint16(i * 2, true) / 1000).toFixed(3);
        document.getElementById(`cell-${i+1}`).textContent = `${val} V`;
    }

    // 2. New Metrics: Battery Health (SoH) - Assumed at offset 39 (uint8)
    const soh = data.getUint8(39);
    document.getElementById('battery-health').textContent = soh;

    // 3. New Metrics: Pack Status - Assumed at offset 40 (uint8)
    // 0 = Idle, 1 = Charging, 2 = Discharging
    const statusVal = data.getUint8(40);
    const statusEl = document.getElementById('pack-status');
    const statusIcon = document.getElementById('status-icon');

    if (statusVal === 1) {
        statusEl.textContent = "CHARGING";
        statusEl.className = "text-lg md:text-xl font-bold data-font text-emerald-400";
        statusIcon.setAttribute('data-lucide', 'zap');
        statusIcon.className = "w-5 h-5 md:w-6 md:h-6 text-emerald-400";
    } else if (statusVal === 2) {
        statusEl.textContent = "DISCHARGING";
        statusEl.className = "text-lg md:text-xl font-bold data-font text-amber-400";
        statusIcon.setAttribute('data-lucide', 'battery-medium');
        statusIcon.className = "w-5 h-5 md:w-6 md:h-6 text-amber-400";
    } else {
        statusEl.textContent = "IDLE";
        statusEl.className = "text-lg md:text-xl font-bold data-font text-slate-400";
        statusIcon.setAttribute('data-lucide', 'power-off');
        statusIcon.className = "w-5 h-5 md:w-6 md:h-6 text-slate-500";
    }

    // Re-draw the icons since we just dynamically swapped the data-lucide attribute
    if (window.lucide) {
        window.lucide.createIcons();
    }

    // 4. New Metrics: Cycles - Assumed at offsets 41 & 43 (uint16 each)
    const chargeCycles = data.getUint16(41, true);
    const dischargeCycles = data.getUint16(43, true);
    
    document.getElementById('charge-cycles').textContent = chargeCycles;
    document.getElementById('discharge-cycles').textContent = dischargeCycles;

    // Visual indicator that data is streaming
    document.getElementById('status-dot').className = "w-2 h-2 rounded-full bg-emerald-500 shadow-[0_0_8px_rgba(16,185,129,0.8)]";
}