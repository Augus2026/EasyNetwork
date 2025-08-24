use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use crate::endpoint::{
    EndpointDeviceInfo,
    ENDPOINT_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SysInfoRequest {
    uuid: String,
    deviceInfo: EndpointDeviceInfo,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
}

#[post("/api/v1/endpoint/sysinfo")]
async fn report_sysinfo(body: web::Json<SysInfoRequest>) -> impl Responder {
    let mut config = ENDPOINT_CONFIG.lock().unwrap();

    let endpoint = config.iter_mut().find(|v| v.uuid == body.uuid);
    if endpoint.is_none() {
        return HttpResponse::NotFound().body("Endpoint not found");
    }

    // 更新ENDPOINT_CONFIG
    let endpoint2 = endpoint.unwrap();
    endpoint2.deviceInfo = body.deviceInfo.clone();
    endpoint2.last_updated = chrono::Utc::now();

    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
    })
}
