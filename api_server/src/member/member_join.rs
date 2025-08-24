use actix_web::{post, web, HttpResponse, Responder};
use chrono::Utc;
use serde::{Deserialize, Serialize};
use std::sync::atomic::{AtomicU8, Ordering};

use crate::member::{
    InterfaceInfo,
    ServerInfo,
    MemberInfo,
    MEMBER_CONFIG,
};

use crate::network::{
    NETWORK_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    data: MemberInfo,
}

#[post("/api/v1/networks/{network_id}/join")]
async fn member_join(
    path: web::Path<String>,
) -> impl Responder {
    let network_id = path.into_inner();

    let mut config = match MEMBER_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    // 查找网络
    let network_config = NETWORK_CONFIG.lock().unwrap();
    let network = network_config.iter().find(|n| n.basic_info.id == network_id);
    match network {
        Some(n) => {
            static IP_COUNTER: AtomicU8 = AtomicU8::new(100);
            let ip_last_octet = IP_COUNTER.fetch_add(1, Ordering::SeqCst);
            let ip_last_octet = if ip_last_octet > 200 { 100 } else { ip_last_octet };
            let ip_address = format!("10.10.10.{}", ip_last_octet);

            // 分配网络成员信息
            let member = MemberInfo {
                id: "1".to_string(),
                name: "member1".to_string(),
                desc: "member1".to_string(),
                auth: "1".to_string(),
                managedIPs: "10.10.10.1".to_string(),
                lastSeen: Utc::now().to_string(),
                version: "1.0.0".to_string(),
                physicalIP: "10.10.10.1".to_string(),
                address: "10.10.10.1".to_string(),
                last_online: Utc::now().to_string(),
                join_time: Utc::now().to_string(),
                online: true,
                interface: InterfaceInfo {
                    name: n.basic_info.name.clone(),
                    desc: format!("{} network", n.basic_info.name),
                    ipv4_address: ip_address,
                    subnet_mask: "24".to_string(),
                    mtu: 1400,
                    domain: "sub1".to_string(),
                    name_server: "114.114.114.114,8.8.8.8".to_string(),
                    search_list: "EasyNetwork.local".to_string(),
                },
                server: ServerInfo {
                    reply_address: "cn-easy-network.com".to_string(),
                    reply_port: "1234".to_string(),
                },
            };

            // 加入网络成员列表
            let members = config.get_mut(&network_id);
            match members {
                Some(m) => {
                    m.push(member.clone());
                }
                None => {
                    return HttpResponse::NotFound().body("No member found for this id");
                }
            }

            HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
                data: member.clone(),
            })
        }
        None => {
            return HttpResponse::NotFound().body("No network found for this id");
        }
    }


}
