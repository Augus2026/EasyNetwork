use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;

use crate::device::{
    DeviceConfig,
    DEVICE_CONFIG,
};

#[derive(Debug, Deserialize, Validate)]
struct OnlineStatusRequest {
    #[validate(length(min = 1, message = "uuid cannot be empty"))]
    uuid: String,
}

#[derive(Debug, Serialize)]
struct OnlineStatusResponse {
    status: String,
}

#[post("/api/v1/device/online_status")]
async fn update_online_status(
    body: web::Json<OnlineStatusRequest>
) -> impl Responder {
    println!("update_online_status {:?}", body);

    let mut config = DEVICE_CONFIG.lock().unwrap();
    let device = config.iter_mut().find(|v| v.uuid == body.uuid);
    match device {
        Some(v) => {
            v.last_updated = chrono::Utc::now();
            return HttpResponse::Ok().json(OnlineStatusResponse {
                status: "success".to_string(),
            });
        }
        None => {
            return HttpResponse::NotFound().body("Device not found");
        }
    }
}
