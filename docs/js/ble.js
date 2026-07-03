export const SERVICE_UUID = "12345678-9abc-def0-1234-56789abcdef1";
export const CHAR_UUID = "12345678-9abc-def0-1234-56789abcdef2";

export async function connectBLE(onDataReceived) {
    const device = await navigator.bluetooth.requestDevice({
        filters: [{ services: [SERVICE_UUID] }],
        optionalServices: [SERVICE_UUID]
    });

    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    const char = await service.getCharacteristic(CHAR_UUID);

    await char.startNotifications();
    char.addEventListener('characteristicvaluechanged', (event) => {
        onDataReceived(event.target.value);
    });
    return device;
}