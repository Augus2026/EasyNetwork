use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use crate::member::{
    MEMBER_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
}

#[post("/api/v1/networks/{network_id}/members/{member_id}/leave")]
async fn member_leave(
    path: web::Path<(String, String)>,
) -> impl Responder {
    let (network_id, member_id) = path.into_inner();

    let mut config = match MEMBER_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => {
            return HttpResponse::InternalServerError().body("Failed to acquire member config lock");
        }
    };

    // 查找并移除成员
    let members = config.get_mut(&network_id);
    match members {
        Some(m) => {
            if let Some(index) = m.iter().position(|m| m.id == member_id) {
                m.remove(index);
                HttpResponse::Ok().json(SuccessResponse {
                    status: "success".to_string(),
                })
            } else {
                HttpResponse::NotFound().body("Member not found")
            }
        }
        None => {
            return HttpResponse::NotFound().body("No member found for this id");
        }
    }
}
