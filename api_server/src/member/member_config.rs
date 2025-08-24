use serde::Serialize;
use serde::Deserialize;
use std::collections::HashMap;
use std::sync::Mutex;
use lazy_static::lazy_static;

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct InterfaceInfo {
    pub name: String,
    pub desc: String,
    pub ipv4_address: String,
    pub subnet_mask: String,
    pub mtu: u32,
    pub domain: String,
    pub name_server: String,
    pub search_list: String,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct ServerInfo {
    pub reply_address: String,
    pub reply_port: String,
}

// member info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct MemberInfo {
    pub id: String,
    pub name: String,
    pub desc: String,
    pub auth: String,
    pub address: String,
    pub managedIPs: String,
    pub lastSeen: String,
    pub version: String,
    pub physicalIP: String,

    // 最后一次在线时间
    pub last_online: String,
    // 加入时间
    pub join_time: String,
    // 在线状态
    pub online: bool,
    
    // 接口信息
    pub interface: InterfaceInfo,
    // 服务器信息
    pub server: ServerInfo,
}

lazy_static! {
    pub static ref MEMBER_CONFIG: Mutex<HashMap<String, Vec<MemberInfo>>> = Mutex::new(HashMap::new());
}
