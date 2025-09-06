use serde::Serialize;
use serde::Deserialize;

use std::collections::HashMap;
use std::sync::Mutex;
use chrono::{DateTime, Utc};
use lazy_static::lazy_static;

// 设备规格
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct DeviceInfo {
    // 设备规格
    pub computer_name: String,
    pub num_processor: String,
    pub memory: String,
    pub product_id: String,
    pub device_id: String,
    
    // Windows规格
    pub user_name: String,
    pub product_name: String,
    pub edition_id: String,
    pub display_version: String,
    pub install_date: String,
    pub build_number: String,
}

// 终端配置
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct DeviceConfig {
    // 设备ID
    pub uuid: String,
    // 设备规格
    pub computer_name: String,
    pub num_processor: String,
    pub memory: String,
    pub product_id: String,
    pub device_id: String,
    // Windows规格
    pub user_name: String,
    pub product_name: String,
    pub edition_id: String,
    pub display_version: String,
    pub install_date: String,
    pub build_number: String,
    // 最新更新时间
    pub last_updated: chrono::DateTime<chrono::Utc>,
}

lazy_static! {
    pub static ref DEVICE_CONFIG: Mutex<Vec<DeviceConfig>> = Mutex::new(Vec::new());
}
