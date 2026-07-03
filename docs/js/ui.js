export function updateDashboard(data) {
    document.getElementById('pack-voltage').textContent = (data.getUint32(32, true)/1000).toFixed(2) + " V";
    document.getElementById('soc').textContent = data.getUint8(38) + "%";
    document.getElementById('cell-delta').textContent = data.getUint16(36, true) + " mV";
    
    for(let i=0; i<12; i++) {
        const val = (data.getUint16(i*2, true)/1000).toFixed(3);
        document.getElementById(`cell-${i+1}`).textContent = `${val} V`;
    }
    document.getElementById('status-dot').className = "w-2 h-2 rounded-full bg-emerald-500";
}