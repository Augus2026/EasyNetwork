use serde::Serialize;
use serde::Deserialize;

use std::collections::HashMap;
use std::sync::Mutex;
use chrono::{DateTime, Utc};
use lazy_static::lazy_static;

// 操作系统信息
#[derive(Debug, Clone, Deserialize, Serialize)]

pub struct EndpointDeviceInfo {
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
pub struct EndpointConfig {
    pub uuid: String,
    pub deviceInfo: EndpointDeviceInfo,
    pub last_updated: chrono::DateTime<chrono::Utc>,
}

lazy_static! {
    pub static ref ENDPOINT_CONFIG: Mutex<Vec<EndpointConfig>> = Mutex::new(Vec::new());
}
