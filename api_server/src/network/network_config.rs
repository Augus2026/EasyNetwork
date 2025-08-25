use serde::Serialize;
use serde::Deserialize;

use std::sync::Mutex;
use lazy_static::lazy_static;

// basic info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct BasicInfo {
    pub id: String,
    pub name: String,
    pub desc: String,
    pub devices: String,
    pub created: String,
    pub is_private: bool,
}

// route info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct RouteInfo {
    pub dest: String,
    pub netmask: String,
    pub gateway: String,
    pub metric: i32,
}

// dhcp info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct DhcpInfo {
    pub alloc_type: String,
    pub range_start: String,
    pub range_end: String,
    pub selected_range: String,
}

// dns info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct DnsInfo {
    pub search_domain: String,
    pub server_address: String,
}

// server info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct ServerInfo {
    pub server_address: String,
    pub server_port: String,
}

// member info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct MemberInfo {
    // 基本信息
    pub id: String,
    pub name: String,
    pub desc: String,
    pub auth: String,
    pub address: String,
    pub managed_ips: String,
    pub last_seen: String,
    pub version: String,
    pub physical_ip: String,
}

// network
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct NetworkConfig {
    // basic info
    pub basic_info: BasicInfo,
    // route info
    pub route_info: Vec<RouteInfo>,
    // dhcp info
    pub dhcp_info: DhcpInfo,
    // dns info
    pub dns_info: DnsInfo,
    // server info
    pub server_info: ServerInfo,
    // member info
    pub member_info: Vec<MemberInfo>,
}

lazy_static! {
    pub static ref NETWORK_CONFIG: Mutex<Vec<NetworkConfig>> = Mutex::new(Vec::new());
}
