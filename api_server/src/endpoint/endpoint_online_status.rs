use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;

use crate::endpoint::{
    EndpointDeviceInfo,
    EndpointConfig,
    ENDPOINT_CONFIG,
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

#[post("/api/v1/endpoint/online_status")]
async fn update_online_status(
    body: web::Json<OnlineStatusRequest>
) -> impl Responder {
    println!("update_online_status {:?}", body);

    let mut config = ENDPOINT_CONFIG.lock().unwrap();
    let endpoint = config.iter_mut().find(|v| v.uuid == body.uuid);
    match endpoint {
        Some(v) => {
            v.last_updated = chrono::Utc::now();
            return HttpResponse::Ok().json(OnlineStatusResponse {
                status: "success".to_string(),
            });
        }
        None => {
            return HttpResponse::NotFound().body("Endpoint not found");
        }
    }
}
