use serde::Serialize;
use serde::Deserialize;
use std::collections::HashMap;
use std::sync::Mutex;
use lazy_static::lazy_static;

use crate::network::network_config::{
    NetworkConfig,
};

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
    // 网络信息
    pub network: NetworkConfig,
}

lazy_static! {
    pub static ref MEMBER_CONFIG: Mutex<HashMap<String, Vec<MemberInfo>>> = Mutex::new(HashMap::new());
}
