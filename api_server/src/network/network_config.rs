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
    pub domain: String,
    pub name_server: String,
    pub search_list: String,
}

// server info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct ServerInfo {
    pub reply_address: String,
    pub reply_port: String,
}

// cert info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct CertInfo {
    pub ca_cert: String,
    pub ca_key: String,
    pub server_cert: String,
    pub server_key: String,
}

// member info
#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct MemberInfo {
    // 基本信息
    pub id: String,
    pub name: String,
    pub desc: String,
    pub auth: String,
    pub mac_address: String,
    pub ipv4_address: String,
    pub subnet_mask: String,
    pub mtu: i32,
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
    // cert info
    pub cert_info: CertInfo,
}

lazy_static! {
    pub static ref NETWORK_CONFIG: Mutex<Vec<NetworkConfig>> = Mutex::new(Vec::new());
}
