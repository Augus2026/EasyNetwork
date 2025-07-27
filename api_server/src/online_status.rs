use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;
use crate::config::{EndpointStatus, EndpointDeviceInfo, ENDPOINT_STATUS};

#[derive(Debug, Deserialize, Validate)]
struct OnlineStatusRequest {
    #[validate(length(min = 1, message = "uuid cannot be empty"))]
    uuid: String,
}

#[derive(Debug, Serialize)]
struct OnlineStatusResponse {
    status: String,
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

#[post("/api/v1/online_status")]
async fn update_online_status(body: web::Json<OnlineStatusRequest>) -> impl Responder {
    println!("Received update online status request: {:?}", body);

    if let Err(validation_errors) = body.validate() {
        return HttpResponse::BadRequest().json(ErrorResponse {
            status: "error".to_string(),
            error: ErrorDetail {
                code: "INVALID_REQUEST".to_string(),
                message: validation_errors.to_string(),
            },
        });
    }

    // 更新ENDPOINT_STATUS中的last_updated
    let mut status_map = ENDPOINT_STATUS.lock().unwrap();
    if let Some(status) = status_map.get_mut(&body.uuid) {
        status.last_updated = chrono::Utc::now();
    } else {
        println!("UUID not found in ENDPOINT_STATUS: {:?}", body.uuid);
    }

    HttpResponse::Ok().json(OnlineStatusResponse {
        status: "success".to_string(),
    })
}
