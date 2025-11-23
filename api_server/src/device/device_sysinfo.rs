use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use log::{info, error};

use crate::device::{
    DeviceInfo,
    DeviceConfig,
    DEVICE_CONFIG,
};

#[derive(Debug, Deserialize, Serialize)]
struct SysInfoRequest {
    uuid: String,
    device_info: DeviceInfo,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
    status: String,
}

#[post("/api/v1/device/sysinfo")]
async fn report_device_sysinfo(
    body: web::Json<SysInfoRequest>
) -> impl Responder {
    info!("report_device_sysinfo {:?}", body);

    let mut config = DEVICE_CONFIG.lock().unwrap();
    let device = config.iter_mut().find(|v| v.uuid == body.uuid);
    match device {
        Some(v) => {
            v.computer_name = body.device_info.computer_name.clone();
            v.num_processor = body.device_info.num_processor.clone();
            v.memory = body.device_info.memory.clone();
            v.product_id = body.device_info.product_id.clone();
            v.device_id = body.device_info.device_id.clone();
            v.user_name = body.device_info.user_name.clone();
            v.product_name = body.device_info.product_name.clone();
            v.edition_id = body.device_info.edition_id.clone();
            v.display_version = body.device_info.display_version.clone();
            v.install_date = body.device_info.install_date.clone();
            v.build_number = body.device_info.build_number.clone();
            v.last_updated = chrono::Utc::now();
            return HttpResponse::Ok().json(SuccessResponse {
                status: "success".to_string(),
            });
        }
        None => {
            config.push(DeviceConfig {
                uuid: body.uuid.clone(),
                computer_name: body.device_info.computer_name.clone(),
                num_processor: body.device_info.num_processor.clone(),
                memory: body.device_info.memory.clone(),
                product_id: body.device_info.product_id.clone(),
                device_id: body.device_info.device_id.clone(),
                user_name: body.device_info.user_name.clone(),
                product_name: body.device_info.product_name.clone(),
                edition_id: body.device_info.edition_id.clone(),
                display_version: body.device_info.display_version.clone(),
                install_date: body.device_info.install_date.clone(),
                build_number: body.device_info.build_number.clone(),
                last_updated: chrono::Utc::now(),
            });
            return HttpResponse::Ok().json(SuccessResponse {
                status: "update".to_string(),
            });
        }
    }
}
