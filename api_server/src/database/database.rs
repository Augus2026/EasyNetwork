use rusqlite::{Connection, Result};
use std::sync::Mutex;
use lazy_static::lazy_static;
use tokio::time::{interval, Duration};

use crate::device::{
    DEVICE_CONFIG,
};

use crate::network::{
    NETWORK_CONFIG,
};

use crate::database::device::{
    init_device_database,
    load_all_devices,
    save_all_devices,
};

use crate::database::network::{
    init_network_database,
    load_all_networks,
    save_network_config,
};

lazy_static! {
    pub static ref DB_CONNECTION: Mutex<Option<Connection>> = Mutex::new(None);
}

pub fn init_database(db_path: String) -> Result<()> {
    let conn = Connection::open(db_path)?;
    init_device_database(&conn)?;
    init_network_database(&conn)?;
    let mut db_guard = DB_CONNECTION.lock().unwrap();
    *db_guard = Some(conn);
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

    let networks = load_all_networks().map_err(|e| {
        eprintln!("Failed to load networks from database: {}", e);
        e
    })?;
    let mut config = NETWORK_CONFIG.lock().unwrap();
    *config = networks;
    println!("Successfully loaded {} networks from database", config.len());
    
    Ok(())
}

pub async fn save_all_config() {

    let config = DEVICE_CONFIG.lock().unwrap();
    if let Err(e) = crate::database::save_all_devices(&config) {
        eprintln!("Auto-save failed: {}", e);
    } else {
        println!("Auto-saved {} devices to database", config.len());
    }

    let config = NETWORK_CONFIG.lock().unwrap();
    if let Err(e) = crate::database::save_all_networks(&config) {
        eprintln!("Auto-save failed: {}", e);
    } else {
        println!("Auto-saved {} networks to database", config.len());
    }
}

pub async fn start_auto_save() {
    let mut interval = interval(Duration::from_secs(300));
    
    loop {
        interval.tick().await;
        save_all_config().await;
    }
}
