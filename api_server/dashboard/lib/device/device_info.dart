import 'package:flutter/material.dart';

class DeviceInfo {
  String uuid;

  // 设备规格
  String computerName;
  String numProcessor;
  String memory;
  String productId;
  String deviceId;
  
  // Windows规格
  String userName;
  String productName;
  String editionId;
  String displayVersion;
  String installDate;
  String buildNumber;

  String lastUpdate;

  DeviceInfo({
    required this.uuid,
    required this.computerName,
    required this.numProcessor,
    required this.memory,
    required this.productId,
    required this.deviceId,
    required this.userName,
    required this.productName,
    required this.editionId,
    required this.displayVersion,
    required this.installDate,
    required this.buildNumber,
    required this.lastUpdate,
  });

  factory DeviceInfo.fromJson(Map<String, dynamic> json) {
    return DeviceInfo(
      uuid: json['uuid'],
      computerName: json['computer_name'],
      numProcessor: json['num_processor'],
      memory: json['memory'],
      productId: json['product_id'],
      deviceId: json['device_id'],
      userName: json['user_name'],
      productName: json['product_name'],
      editionId: json['edition_id'],
      displayVersion: json['display_version'],
      installDate: json['install_date'],
      buildNumber: json['build_number'],
      lastUpdate: json['last_update'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'uuid': uuid,
      'computer_name': computerName,
      'num_processor': numProcessor,
      'memory': memory,
      'product_id': productId,
      'device_id': deviceId,
      'user_name': userName,
      'product_name': productName,
      'edition_id': editionId,
      'display_version': displayVersion,
      'install_date': installDate,
      'build_number': buildNumber,
      'last_update': lastUpdate,
    };
  }
}