use actix_web::{get, web, HttpResponse, Responder};
use chrono::{DateTime, Utc};
use serde::Serialize;

use crate::config::NETWORK_STATUS;

#[derive(Debug, Serialize)]
struct Member {
    member_id: String,
    online: bool,
    joined_at: DateTime<Utc>,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<Member>,
}

#[derive(Debug, Serialize)]
struct ErrorResponse {
    status: String,
}

#[get("/api/v1/networks/{network_id}/members")]
async fn get_network_members(path: web::Path<String>) -> impl Responder {
    let network_id = path.into_inner();
    println!("Received request for network members: {}", network_id);

    // 检查网络是否存在
    let network_status = NETWORK_STATUS.lock().unwrap();
    if let Some(devices) = network_status.get(&network_id) {
        let members = devices.iter().map(|(device_id, status)| Member {
            member_id: device_id.clone(),
            online: (chrono::Utc::now() - status.last_updated) < chrono::Duration::seconds(60),
            joined_at: status.last_updated,
        }).collect::<Vec<_>>();

        HttpResponse::Ok().json(SuccessResponse {
            status: "success".to_string(),
            data: members,
        })
    } else {
        HttpResponse::Ok().json(SuccessResponse {
            status: "success".to_string(),
            data: vec![],
        })
    }
}
