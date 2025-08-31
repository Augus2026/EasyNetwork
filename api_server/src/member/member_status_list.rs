use actix_web::{get, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use chrono::{Utc, DateTime, Duration};

use crate::network::{
    NETWORK_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct MemberStatusInfo {
    name: String,
    online: bool,
    status: String,
}

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<MemberStatusInfo>,
}

#[get("api/v1/networks/{network_id}/members/{member_id}/status")]
pub async fn get_network_member_status(
    path: web::Path<(String, String)>,
) -> impl Responder {
    let (network_id, member_id) = path.into_inner();

    let mut config = match NETWORK_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    let network = config.iter_mut().find(|v| v.basic_info.id == network_id);
    match network {
        Some(network) => {
            // 更新成员在线时间
            let member = network.member_info.iter_mut().find(|m| m.id == member_id);
            if member.is_none() {
                return HttpResponse::NotFound().body("Member not found");
            }
            let member = member.unwrap();
            member.last_seen = Utc::now().to_string();

            let data: Vec<MemberStatusInfo> = network.member_info.iter().map(|m| {
                let last_seen_time: DateTime<Utc> = DateTime::parse_from_rfc3339(&m.last_seen)
                    .unwrap_or(Utc::now().into())
                    .with_timezone(&Utc);
                let now = Utc::now();
                let time_diff = now - last_seen_time;
                
                // 判断是否在线（60秒内）
                let online = time_diff.num_seconds() <= 60;
                
                // 生成状态描述
                let status = if online {
                    "在线".to_string()
                } else {
                    match time_diff {
                        diff if diff.num_minutes() < 60 => format!("{}分钟前", diff.num_minutes()),
                        diff if diff.num_hours() < 24 => format!("{}小时前", diff.num_hours()),
                        diff if diff.num_days() < 30 => format!("{}天前", diff.num_days()),
                        _ => "很久以前".to_string()
                    }
                };
                
                MemberStatusInfo {
                    name: m.name.clone(),
                    online,
                    status,
                }
            }).collect();

            // 返回网络成员状态
            return HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
                data: data,
            });
        }
        None => {
            return HttpResponse::InternalServerError().body("Network ID not found");
        }
    }
}