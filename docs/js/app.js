import { connectBLE } from './ble.js';
import { updateDashboard } from './ui.js';

document.getElementById('connect-btn').onclick = async () => {
    try {
        await connectBLE(updateDashboard);
    } catch (err) {
        console.error("BLE Connection Error:", err);
    }
};