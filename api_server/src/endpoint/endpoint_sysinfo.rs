use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use crate::endpoint::{
    EndpointConfig,
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
async fn report_sysinfo(
    body: web::Json<SysInfoRequest>
) -> impl Responder {
    println!("report_sysinfo {:?}", body);

    let mut config = ENDPOINT_CONFIG.lock().unwrap();
    let endpoint = config.iter_mut().find(|v| v.uuid == body.uuid);
    match endpoint {
        Some(v) => {
            v.deviceInfo = body.deviceInfo.clone();
            v.last_updated = chrono::Utc::now();
            return HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
            });
        }
        None => {
            config.push(EndpointConfig {
                uuid: body.uuid.clone(),
                deviceInfo: body.deviceInfo.clone(),
                last_updated: chrono::Utc::now(),
            });
            return HttpResponse::Ok().json(SuccessResponse {
                status: "update".to_string(),
            });
        }
    }
}
