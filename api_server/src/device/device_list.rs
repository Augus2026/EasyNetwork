use actix_web::{get, HttpResponse, Responder};
use serde::Serialize;

use crate::device::{
    DeviceConfig,
    DEVICE_CONFIG
};

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
    data: Vec<DeviceConfig>,
}

#[get("/api/v1/devices")]
async fn get_all_devices() -> impl Responder {
    let config = DEVICE_CONFIG.lock().unwrap();
    let devices = config.iter().cloned().collect::<Vec<_>>();
    
    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
        data: devices,
    })
}
