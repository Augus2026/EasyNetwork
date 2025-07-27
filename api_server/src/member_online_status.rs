use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;
use std::collections::HashMap;

use crate::config::{
    NetworkMemberStatus,
    NETWORK_STATUS,
};

#[derive(Debug, Deserialize, Validate)]
struct CheckOnlineRequest {
    #[validate(length(min = 1, message = "network_name cannot be empty"))]
    network_name: String,

    #[validate(length(min = 1, message = "device_id cannot be empty"))]
    device_id: String,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    online_status: String,
}

#[derive(Debug, Serialize)]
struct ErrorResponse {
    status: String,
    error: ErrorDetail,
}

#[derive(Debug, Serialize)]
struct ErrorDetail {
    code: String,
    message: String,
}

#[post("/api/v1/member_online_status")]
async fn update_member_online_status(body: web::Json<CheckOnlineRequest>) -> impl Responder {
    println!("Received member online status: {:?}", body);
    if let Err(validation_errors) = body.validate() {
        return HttpResponse::BadRequest().json(ErrorResponse {
            status: "error".to_string(),
            error: ErrorDetail {
                code: "INVALID_REQUEST".to_string(),
                message: validation_errors.to_string(),
            },
        });
    }
    let is_online = update_device_online(&body.network_name, &body.device_id).await;
    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        online_status: if is_online { "online" } else { "offline" }.to_string(),
    })
}

async fn update_device_online(network_name: &str, device_id: &str) -> bool {
    let mut network_status = NETWORK_STATUS.lock().unwrap();
    let network: &mut HashMap<String, NetworkMemberStatus> = network_status.entry(network_name.to_string()).or_insert_with(HashMap::new);
    let network_status = network.entry(device_id.to_string()).or_insert_with(|| NetworkMemberStatus {
        last_updated: chrono::Utc::now(),
    });
    network_status.last_updated = chrono::Utc::now();
    true
}
