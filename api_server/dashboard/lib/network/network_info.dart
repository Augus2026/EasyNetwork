import 'package:flutter/material.dart';

class BasicInfo {
  String id;
  String name;
  String desc;
  String devices;
  String created;
  bool isPrivate = false;

  BasicInfo({
    required this.id,
    required this.name,
    required this.desc,
    required this.devices,
    required this.created,
    required this.isPrivate});

  factory BasicInfo.fromJson(Map<String, dynamic> json) {
    return BasicInfo(
      id: json['id'],
      name: json['name'],
      desc: json['desc'],
      devices: json['devices'],
      created: json['created'],
      isPrivate: json['is_private'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      'desc': desc,
      'devices': devices,
      'created': created,
      'is_private': isPrivate,
    };
  }

}

class RouteInfo {
  String dest;
  String netmask;
  String gateway;
  int metric = 0;

  RouteInfo({
    required this.dest,
    required this.netmask,
    required this.gateway,
    required this.metric,
  });

  factory RouteInfo.fromJson(Map<String, dynamic> json) {
    return RouteInfo(
      dest: json['dest'],
      netmask: json['netmask'],
      gateway: json['gateway'],
      metric: json['metric'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'dest': dest,
      'netmask': netmask,
      'gateway': gateway,
      'metric': metric,
    };
  }

}

class DhcpInfo {
  String allocType;
  String rangeStart;
  String rangeEnd;
  String selectedRange;

  DhcpInfo({
    required this.allocType,
    required this.rangeStart,
    required this.rangeEnd,
    required this.selectedRange});

  factory DhcpInfo.fromJson(Map<String, dynamic> json) {
    return DhcpInfo(
      allocType: json['alloc_type'],
      rangeStart: json['range_start'],
      rangeEnd: json['range_end'],
      selectedRange: json['selected_range'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'alloc_type': allocType,
      'range_start': rangeStart,
      'range_end': rangeEnd,
      'selected_range': selectedRange,
    };
  }

}

class DnsInfo {
  String domain;
  String nameServer;
  String searchList;

  DnsInfo({
    required this.domain,
    required this.nameServer,
    required this.searchList});

  factory DnsInfo.fromJson(Map<String, dynamic> json) {
    return DnsInfo(
      domain: json['domain'],
      nameServer: json['name_server'],
      searchList: json['search_list'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'domain': domain,
      'name_server': nameServer,
      'search_list': searchList,
    };
  }

}

class ServerInfo {
  String replyAddress;
  String replyPort;

  ServerInfo({
    required this.replyAddress,
    required this.replyPort});

  factory ServerInfo.fromJson(Map<String, dynamic> json) {
    return ServerInfo(
      replyAddress: json['reply_address'],
      replyPort: json['reply_port'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'reply_address': replyAddress,
      'reply_port': replyPort,
    };
  }
}

class CertInfo {
  String caCert;
  String caKey;
  String serverCert;
  String serverKey;

  CertInfo({
    required this.caCert,
    required this.caKey,
    required this.serverCert,
    required this.serverKey,
  });

  factory CertInfo.fromJson(Map<String, dynamic> json) {
    return CertInfo(
      caCert: json['ca_cert'],
      caKey: json['ca_key'],
      serverCert: json['server_cert'],
      serverKey: json['server_key'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'ca_cert': caCert,
      'ca_key': caKey,
      'server_cert': serverCert,
      'server_key': serverKey,
    };
  }
}

class NetworkInfo {
  BasicInfo basic;
  List<RouteInfo> routes;
  DhcpInfo dhcp;
  DnsInfo dns;
  ServerInfo server;
  CertInfo cert;

  NetworkInfo({
    required this.basic,
    required this.routes,
    required this.dhcp,
    required this.dns,
    required this.server,
    required this.cert,
  });

  factory NetworkInfo.fromJson(Map<String, dynamic> json) {
    return NetworkInfo(
      basic: BasicInfo.fromJson(json['basic_info']),
      routes: (json['route_info'] as List).map((e) => RouteInfo.fromJson(e)).toList(),
      dhcp: DhcpInfo.fromJson(json['dhcp_info']),
      dns: DnsInfo.fromJson(json['dns_info']),
      server: ServerInfo.fromJson(json['server_info']),
      cert: CertInfo.fromJson(json['cert_info']),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'basic_info': basic.toJson(),
      'route_info': routes.map((e) => e.toJson()).toList(),
      'dhcp_info': dhcp.toJson(),
      'dns_info': dns.toJson(),
      'server_info': server.toJson(),
      'cert_info': cert.toJson(),
    };
  }

}

class Member {
  String id;
  String name;
  String desc;
  String auth;
  String macAddress;
  String ipAddress;
  String subnetMask;
  int mtu;
  String lastSeen;
  String version;
  String physicalIP;

  Member({
    required this.id,
    required this.name,
    required this.desc,
    required this.auth,
    required this.macAddress,
    required this.ipAddress,
    required this.subnetMask,
    required this.mtu,
    required this.lastSeen,
    required this.version,
    required this.physicalIP,
  });

  factory Member.fromJson(Map<String, dynamic> json) {
    return Member(
      id: json['id'],
      name: json['name'],
      desc: json['desc'],
      auth: json['auth'],
      macAddress: json['mac_address'],
      ipAddress: json['ipv4_address'],
      subnetMask: json['subnet_mask'],
      mtu: json['mtu'],
      lastSeen: json['last_seen'],
      version: json['version'],
      physicalIP: json['physical_ip'],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      'desc': desc,
      'auth': auth,
      'mac_address': macAddress,
      'ipv4_address': ipAddress,
      'subnet_mask': subnetMask,
      'mtu': mtu,
      'last_seen': lastSeen,
      'version': version,
      'physical_ip': physicalIP,
    };
  }

}
