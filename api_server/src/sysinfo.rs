use actix_web::{post, web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};
use validator::Validate;
use crate::config::{EndpointStatus, EndpointDeviceInfo, ENDPOINT_STATUS};

#[derive(Debug, Deserialize, Validate)]
struct SysInfoRequest {
    #[validate(length(min = 1, message = "uuid cannot be empty"))]
    uuid: String,
    
    #[validate(nested)]
    deviceInfo: DeviceInfo,
}

#[derive(Debug, Deserialize, Validate)]
struct DeviceInfo {
    // Device specifications
    #[validate(length(min = 1, message = "computer_name cannot be empty"))]
    computer_name: String,
    
    #[validate(length(min = 1, message = "num_processor cannot be empty"))]
    num_processor: String,
    
    #[validate(length(min = 1, message = "memory cannot be empty"))]
    memory: String,
    
    #[validate(length(min = 1, message = "product_id cannot be empty"))]
    product_id: String,
    
    #[validate(length(min = 1, message = "device_id cannot be empty"))]
    device_id: String,
    
    // Windows specifications
    #[validate(length(min = 1, message = "user_name cannot be empty"))]
    user_name: String,
    
    #[validate(length(min = 1, message = "product_name cannot be empty"))]
    product_name: String,
    
    #[validate(length(min = 1, message = "edition_id cannot be empty"))]
    edition_id: String,
    
    #[validate(length(min = 1, message = "display_version cannot be empty"))]
    display_version: String,
    
    #[validate(length(min = 1, message = "install_date cannot be empty"))]
    install_date: String,
    
    #[validate(length(min = 1, message = "build_number cannot be empty"))]
    build_number: String,
}

#[derive(Debug, Serialize)]
struct SuccessResponse {
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

#[post("/api/v1/sysinfo")]
async fn report_sysinfo(body: web::Json<SysInfoRequest>) -> impl Responder {
    println!("Received system info: {:?}", body);

    if let Err(_validation_errors) = body.0.validate() {
        return HttpResponse::BadRequest().json(ErrorResponse {
            status: "error".to_string(),
            error: ErrorDetail {
                code: "400".to_string(),
                message: "Bad Request".to_string(),
            },
        });
    }

    // 更新ENDPOINT_STATUS
    let mut status_map = ENDPOINT_STATUS.lock().unwrap();
    status_map.entry(body.uuid.clone())
        .or_insert_with(|| EndpointStatus {
            uuid: body.uuid.clone(),
            deviceInfo: EndpointDeviceInfo {
                computer_name: body.deviceInfo.computer_name.clone(),
                num_processor: body.deviceInfo.num_processor.clone(),
                memory: body.deviceInfo.memory.clone(),
                product_id: body.deviceInfo.product_id.clone(),
                device_id: body.deviceInfo.device_id.clone(),
                user_name: body.deviceInfo.user_name.clone(),
                product_name: body.deviceInfo.product_name.clone(),
                edition_id: body.deviceInfo.edition_id.clone(),
                display_version: body.deviceInfo.display_version.clone(),
                install_date: body.deviceInfo.install_date.clone(),
                build_number: body.deviceInfo.build_number.clone(),
            },
            last_updated: chrono::Utc::now(),
        });

    HttpResponse::Ok().json(SuccessResponse {
        status: "success".to_string(),
    })
}
