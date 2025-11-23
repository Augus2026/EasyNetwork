use rusqlite::{Connection, Result, params};
use serde_json;

use crate::network::network_config::{
    BasicInfo,
    RouteInfo,
    DhcpInfo,
    DnsInfo,
    ServerInfo,
    MemberInfo,
    CertInfo,
    NetworkConfig
};

use crate::database::DB_CONNECTION;

pub fn init_network_database(conn: &Connection) -> Result<()> {    
    // 创建网络配置表
    conn.execute(
        "CREATE TABLE IF NOT EXISTS network_config (
            id TEXT PRIMARY KEY,
            basic_info TEXT,
            route_info TEXT,
            dhcp_info TEXT,
            dns_info TEXT,
            server_info TEXT,
            member_info TEXT,
            cert_info TEXT
        )",
        [],
    )?;
    
    Ok(())
}

pub fn save_network_config(config: &NetworkConfig) -> Result<()> {
    let db_guard = DB_CONNECTION.lock().unwrap();
    if let Some(conn) = &*db_guard {
        let basic_info_json = serde_json::to_string(&config.basic_info).unwrap();
        let route_info_json = serde_json::to_string(&config.route_info).unwrap();
        let dhcp_info_json = serde_json::to_string(&config.dhcp_info).unwrap();
        let dns_info_json = serde_json::to_string(&config.dns_info).unwrap();
        let server_info_json = serde_json::to_string(&config.server_info).unwrap();
        let member_info_json = serde_json::to_string(&config.member_info).unwrap();
        let cert_info_json = serde_json::to_string(&config.cert_info).unwrap();

        conn.execute(
            "INSERT OR REPLACE INTO network_config 
            (id, basic_info, route_info, dhcp_info, dns_info, server_info, member_info, cert_info)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            params![
                config.basic_info.id.as_str(),
                basic_info_json,
                route_info_json,
                dhcp_info_json,
                dns_info_json,
                server_info_json,
                member_info_json,
                cert_info_json,
            ],
        )?;
    }
    Ok(())
}

pub fn load_all_networks() -> Result<Vec<NetworkConfig>> {
    let db_guard = DB_CONNECTION.lock().unwrap();
    if let Some(conn) = &*db_guard {
        let mut stmt = conn.prepare("SELECT * FROM network_config")?;
        let network_iter = stmt.query_map([], |row| {
            let basic_info: BasicInfo = serde_json::from_str(&row.get::<_, String>(1)?).unwrap();
            let route_info: Vec<RouteInfo> = serde_json::from_str(&row.get::<_, String>(2)?).unwrap();
            let dhcp_info: DhcpInfo = serde_json::from_str(&row.get::<_, String>(3)?).unwrap();
            let dns_info: DnsInfo = serde_json::from_str(&row.get::<_, String>(4)?).unwrap();
            let server_info: ServerInfo = serde_json::from_str(&row.get::<_, String>(5)?).unwrap();
            let member_info: Vec<MemberInfo> = serde_json::from_str(&row.get::<_, String>(6)?).unwrap();
            let cert_info: CertInfo = serde_json::from_str(&row.get::<_, String>(7)?).unwrap();

            Ok(NetworkConfig {
                basic_info,
                route_info,
                dhcp_info,
                dns_info,
                server_info,
                member_info,
                cert_info,
            })
        })?;
        
        let mut networks = Vec::new();
        for network in network_iter {
            networks.push(network?);
        }
        Ok(networks)
    } else {
        Ok(Vec::new())
    }
}

pub fn save_all_networks(networks: &[NetworkConfig]) -> Result<()> {
    for network in networks {
        save_network_config(network)?;
    }
    Ok(())
}