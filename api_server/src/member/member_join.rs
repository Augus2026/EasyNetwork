use actix_web::{post, web, HttpResponse, Responder};
use chrono::Utc;
use serde::{Deserialize, Serialize};
use std::sync::atomic::{AtomicU8, Ordering};
use uuid::Uuid;

use crate::member::{
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

static IP_COUNTER: AtomicU8 = AtomicU8::new(0);

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
            // 生成id
            let member_id = uuid::Uuid::new_v4().to_string();
            // 生成mac地址
            let mac_address = uuid::Uuid::new_v4().to_string();
            // 生成ip地址
            let alloc_type  = n.dhcp_info.alloc_type.clone();
            let ip_address: String;
            match alloc_type.as_str() {
                "easy" => {
                    let range_start = 0;
                    let range_end = 255;
                    let ip_prefix = n.dhcp_info.selected_range.clone();
                    let ip_last_octet = range_start + (IP_COUNTER.fetch_add(1, Ordering::SeqCst) % (range_end - range_start + 1));
                    ip_address = format!("{}.{}", ip_prefix, ip_last_octet);
                }
                "advanced" => {
                    let range_start = n.dhcp_info.range_start.split('.').last().unwrap().parse::<u8>().unwrap();
                    let range_end = n.dhcp_info.range_end.split('.').last().unwrap().parse::<u8>().unwrap();
                    let ip_prefix = &n.dhcp_info.range_start[..n.dhcp_info.range_start.rfind('.').unwrap()];
                    let ip_last_octet = range_start + (IP_COUNTER.fetch_add(1, Ordering::SeqCst) % (range_end - range_start + 1));
                    ip_address = format!("{}.{}", ip_prefix, ip_last_octet);
                }
                _ => {
                    return HttpResponse::InternalServerError().body("Invalid alloc type");
                }
            }
            // 生成认证结果
            let auth = if n.basic_info.is_private { "false" } else { "true" };

            // 分配网络成员信息
            let member = MemberInfo {
                id: member_id.clone(),
                name: "".to_string(),
                desc: "".to_string(),
                auth: auth.to_string(),
                address: mac_address.clone(),
                managed_ips: ip_address.clone(),
                last_seen: Utc::now().to_string(),
                version: "1.0.0".to_string(),
                physical_ip: "10.10.10.1".to_string(),
                network: n.clone(),
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
