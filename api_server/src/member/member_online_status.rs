use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use chrono::Utc;

use crate::member::{
    MEMBER_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SuccessResponse {
    status: String,
}

#[post("/api/v1/networks/{network_id}/members/{member_id}/online_status")]
async fn update_member_online_status(
    path: web::Path<(String, String)>,
) -> impl Responder {
    let (network_id, member_id) = path.into_inner();

    let mut config = match MEMBER_CONFIG.lock() {
        Ok(guard) => guard,
        Err(_) => return HttpResponse::InternalServerError().body("Failed to acquire member config lock"),
    };

    let members = config.get_mut(&network_id);
    match members {
        Some(m) => {
            let member = m.iter_mut().find(|m| m.id == member_id);
            if member.is_none() {
                return HttpResponse::NotFound().body("Member not found");
            }
            let member = member.unwrap();
            member.last_seen = Utc::now().to_string();
            HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
            })
        }
        None => {
            return HttpResponse::NotFound().body("No member found for this id");
        }
    }
}
