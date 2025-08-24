use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;

use crate::endpoint::{
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
    let mut config = ENDPOINT_CONFIG.lock().unwrap();

    let endpoint = config.iter_mut().find(|v| v.uuid == body.uuid);
    if endpoint.is_none() {
        return HttpResponse::NotFound().body("Endpoint not found");
    }
    endpoint.unwrap().last_updated = chrono::Utc::now();
    HttpResponse::Ok().json(OnlineStatusResponse {
        status: "success".to_string(),
    })
}
