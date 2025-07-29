use serde::Serialize;
use std::collections::HashMap;
use std::sync::Mutex;
use chrono::{DateTime, Utc};
use lazy_static::lazy_static;

// 操作系统信息
#[derive(Debug, Clone, Serialize)]
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

// 终端状态
#[derive(Debug, Clone, Serialize)]
pub struct EndpointStatus {
    pub uuid: String,
    pub deviceInfo: EndpointDeviceInfo,
    pub last_updated: chrono::DateTime<chrono::Utc>,
}

#[derive(Debug, Clone, Serialize)]
pub struct InterfaceInfo {
    pub name: String,
    pub desc: String,
    // interface settings
    pub ipv4_address: String,
    pub subnet_mask: String,
    // mtu settings
    pub mtu: i32,
    // dns settings
    pub domain: String,
    pub name_server: String,
    pub search_list: String,
}

#[derive(Debug, Clone, Serialize)]
pub struct ServerInfo {
    pub reply_address: String,
    pub reply_port: String,
    pub expires_at: DateTime<Utc>,
    pub expires: String,
}

// 网络成员状态
#[derive(Debug, Clone, Serialize)]
pub struct NetworkMemberStatus {
    // 成员最后更新时间
    pub last_updated: chrono::DateTime<chrono::Utc>,
    // 接口信息
    pub interface: InterfaceInfo,
    // 服务器信息
    pub server: ServerInfo,
}

lazy_static! {
    pub static ref NETWORK_STATUS: Mutex<HashMap<String, HashMap<String, NetworkMemberStatus>>> = Mutex::new(HashMap::new());
    pub static ref ENDPOINT_STATUS: Mutex<HashMap<String, EndpointStatus>> = Mutex::new(HashMap::new());
}

// 获取所有终端信息
pub fn get_all_endpoints() -> Vec<EndpointStatus> {
    let status_map = ENDPOINT_STATUS.lock().unwrap();
    status_map.values().cloned().collect()
}

// 获取所有网络成员
pub fn get_all_network_members(network_name: &str) -> Option<HashMap<String, NetworkMemberStatus>> {
    let status_map = NETWORK_STATUS.lock().unwrap();
    status_map.get(network_name).cloned()
}