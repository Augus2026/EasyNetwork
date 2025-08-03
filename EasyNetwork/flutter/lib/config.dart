import 'dart:convert';
import 'dart:io';
import 'package:shared_preferences/shared_preferences.dart';

Future<String> get_api_server_address() async {
  String server_address = "cn-easy-network.com";
  String server_port = "1001";

  SharedPreferences prefs = await SharedPreferences.getInstance();
  bool forceUseApiServer = prefs.getBool('forceUseApiServer') ?? false;
  if(forceUseApiServer) {
    server_address = prefs.getString('apiServerAddress') ?? '';
    server_port = prefs.getInt('apiServerPort')?.toString() ?? '';
  }
  return '$server_address:$server_port';
}

Future<String> get_reply_server_address() async {
  String server_address = "";
  String server_port = "";

  SharedPreferences prefs = await SharedPreferences.getInstance();
  bool forceUseReplyServer = prefs.getBool('forceUseReplyServer') ?? false;
  if(forceUseReplyServer) {
    server_address = prefs.getString('replyServerAddress') ?? '';
    server_port = prefs.getInt('replyServerPort')?.toString() ?? '';
  }

  if(server_address == "" || server_port == "") {
    return "";
  }
  return '$server_address:$server_port';
}



