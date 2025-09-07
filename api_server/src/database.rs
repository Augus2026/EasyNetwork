use rusqlite::{Connection, Result, params};
use std::sync::Mutex;
use lazy_static::lazy_static;
use tokio::time::{interval, Duration};

use crate::device::{
    DeviceConfig,
    DEVICE_CONFIG
};

lazy_static! {
    static ref DB_CONNECTION: Mutex<Option<Connection>> = Mutex::new(None);
}

pub fn init_database() -> Result<()> {
    let conn = Connection::open("device_config.db")?;
    
    conn.execute(
        "CREATE TABLE IF NOT EXISTS device_config (
            uuid TEXT PRIMARY KEY,
            computer_name TEXT,
            num_processor TEXT,
            memory TEXT,
            product_id TEXT,
            device_id TEXT,
            user_name TEXT,
            product_name TEXT,
            edition_id TEXT,
            display_version TEXT,
            install_date TEXT,
            build_number TEXT,
            last_updated TEXT
        )",
        [],
    )?;
    
    let mut db_guard = DB_CONNECTION.lock().unwrap();
    *db_guard = Some(conn);
    
    Ok(())
}

pub fn save_device_config(config: &DeviceConfig) -> Result<()> {
    let db_guard = DB_CONNECTION.lock().unwrap();
    if let Some(conn) = &*db_guard {
        conn.execute(
            "INSERT OR REPLACE INTO device_config 
            (uuid, computer_name, num_processor, memory, product_id, device_id, 
             user_name, product_name, edition_id, display_version, install_date, 
             build_number, last_updated)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            params![
                config.uuid,
                config.computer_name,
                config.num_processor,
                config.memory,
                config.product_id,
                config.device_id,
                config.user_name,
                config.product_name,
                config.edition_id,
                config.display_version,
                config.install_date,
                config.build_number,
                config.last_updated.to_rfc3339()
            ],
        )?;
    }
    Ok(())
}

pub fn load_all_devices() -> Result<Vec<DeviceConfig>> {
    let db_guard = DB_CONNECTION.lock().unwrap();
    if let Some(conn) = &*db_guard {
        let mut stmt = conn.prepare("SELECT * FROM device_config")?;
        let device_iter = stmt.query_map([], |row| {
            Ok(DeviceConfig {
                uuid: row.get(0)?,
                computer_name: row.get(1)?,
                num_processor: row.get(2)?,
                memory: row.get(3)?,
                product_id: row.get(4)?,
                device_id: row.get(5)?,
                user_name: row.get(6)?,
                product_name: row.get(7)?,
                edition_id: row.get(8)?,
                display_version: row.get(9)?,
                install_date: row.get(10)?,
                build_number: row.get(11)?,
                last_updated: chrono::DateTime::parse_from_rfc3339(&row.get::<_, String>(12)?)
                    .unwrap()
                    .with_timezone(&chrono::Utc),
            })
        })?;
        
        let mut devices = Vec::new();
        for device in device_iter {
            devices.push(device?);
        }
        Ok(devices)
    } else {
        Ok(Vec::new())
    }
}

pub fn save_all_devices(devices: &[DeviceConfig]) -> Result<()> {
    for device in devices {
        save_device_config(device)?;
    }
    Ok(())
}

pub async fn load_all_config() -> Result<()> {
    let devices = load_all_devices().map_err(|e| {
        eprintln!("Failed to load devices from database: {}", e);
        e
    })?;
    
    let mut config = DEVICE_CONFIG.lock().unwrap();
    *config = devices;
    
    println!("Successfully loaded {} devices from database", config.len());
    
    if config.is_empty() {
        println!("No devices found in database");
    } else {
        println!("Device details:");
        for (i, device) in config.iter().enumerate() {
            println!("  Device {}:", i + 1);
            println!("    UUID: {}", device.uuid);
            println!("    Computer Name: {}", device.computer_name);
            println!("    Processors: {}", device.num_processor);
            println!("    Memory: {}", device.memory);
            println!("    Last Updated: {}", device.last_updated.format("%Y-%m-%d %H:%M:%S"));
        }
    }
    
    Ok(())
}

pub async fn save_all_config() {

    let config = DEVICE_CONFIG.lock().unwrap();
    if let Err(e) = crate::database::save_all_devices(&config) {
        eprintln!("Auto-save failed: {}", e);
    } else {
        println!("Auto-saved {} devices to database", config.len());
    }
}

pub async fn start_auto_save() {
    let mut interval = interval(Duration::from_secs(5));
    
    loop {
        interval.tick().await;
        save_all_config().await;
    }
}
