use std::collections::HashMap;
use std::sync::Mutex;
use lazy_static::lazy_static;

// 操作系统信息
#[derive(Debug, Clone)]
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
pub struct EndpointStatus {
    pub uuid: String,
    pub deviceInfo: EndpointDeviceInfo,
    pub last_updated: chrono::DateTime<chrono::Utc>,
}

// 网络成员状态
pub struct NetworkMemberStatus {
    // 成员最后更新时间
    pub last_updated: chrono::DateTime<chrono::Utc>,

    // 接口信息

    // 路由信息

}

lazy_static! {
    pub static ref NETWORK_STATUS: Mutex<HashMap<String, HashMap<String, NetworkMemberStatus>>> = Mutex::new(HashMap::new());
    pub static ref ENDPOINT_STATUS: Mutex<HashMap<String, EndpointStatus>> = Mutex::new(HashMap::new());
}

// 获取所有终端信息
// 获取所有网络成员状态
