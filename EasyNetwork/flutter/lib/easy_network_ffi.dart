import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'generated/easy_network_bindings.dart';
import 'config.dart';

class EasyNetworkFFI {
  late final EasyNetworkBindings _bindings;
  static EasyNetworkFFI? _instance;

  EasyNetworkFFI._internal() {
    final nativeLib = () {
      if (Platform.isWindows) {
        return DynamicLibrary.open('libEasyNetwork.dll');
      } else if (Platform.isAndroid) {
        return DynamicLibrary.open('libnative.so');
      } else if (Platform.isLinux) {
        return DynamicLibrary.open('libEasyNetwork.so');
      } else if (Platform.isMacOS) {
        return DynamicLibrary.open('libEasyNetwork.dylib');
      }
      return DynamicLibrary.process();
    }();
    _bindings = EasyNetworkBindings(nativeLib);
  }

  factory EasyNetworkFFI() {
    return _instance ??= EasyNetworkFFI._internal();
  }

  Future<void> setServer(String replyAddress, int replyPort) async {
    String addr = await get_reply_server_address();
    if(addr != "") {
      List<String> list = addr.split(":");
      replyAddress = list[0];
      replyPort = int.parse(list[1]);
      print('[EasyNetwork] Force setting server: $replyAddress:$replyPort');
    } else {
      print('[EasyNetwork] Setting server: $replyAddress:$replyPort');
    }
    
    final addressPtr = replyAddress.toNativeUtf8();
    try {
      _bindings.set_server(
        addressPtr as Pointer<Char>,
        replyPort,
      );
      print('[EasyNetwork] Server set successfully');
    } catch (e) {
      print('[EasyNetwork] Error setting server: $e');
      rethrow;
    } finally {
      malloc.free(addressPtr);
    }
  }

  Future<void> joinNetwork(
    String ifname,
    String ifdesc,
    String ip,
    String netmask,
    String mtu,
    String domain,
    String nameServer,
    String searchList,
  ) async {
    print('[EasyNetwork] Joining network with interface: $ifname');
    final ifnameNative = ifname.toNativeUtf8();
    final ifdescNative = ifdesc.toNativeUtf8();
    final ipNative = ip.toNativeUtf8();
    final netmaskNative = netmask.toNativeUtf8();
    final mtuNative = mtu.toNativeUtf8();
    final domainNative = domain.toNativeUtf8();
    final nameServerNative = nameServer.toNativeUtf8();
    final searchListNative = searchList.toNativeUtf8();

    try {
      _bindings.join_network(
        ifnameNative as Pointer<Char>,
        ifdescNative as Pointer<Char>,
        ipNative as Pointer<Char>,
        netmaskNative as Pointer<Char>,
        mtuNative as Pointer<Char>,
        domainNative as Pointer<Char>,
        nameServerNative as Pointer<Char>,
        searchListNative as Pointer<Char>,
      );
      print('[EasyNetwork] Network joined successfully');
    } catch (e) {
      print('[EasyNetwork] Error joining network: $e');
      rethrow;
    } finally {
      malloc.free(ifnameNative);
      malloc.free(ifdescNative);
      malloc.free(ipNative);
      malloc.free(netmaskNative);
      malloc.free(mtuNative);
      malloc.free(domainNative);
      malloc.free(nameServerNative);
      malloc.free(searchListNative);
    }
  }

  Future<void> leaveNetwork() async {
    print('[EasyNetwork] Leaving network');
    try {
      _bindings.leave_network();
      print('[EasyNetwork] Network left successfully');
    } catch (e) {
      print('[EasyNetwork] Error leaving network: $e');
      rethrow;
    }
  }

  Future<void> addRoute({
    required String destination,
    required String netmask,
    required String gateway,
    required String metric,
  }) async {
    print('[EasyNetwork] Adding route to $destination');
    final destPtr = destination.toNativeUtf8();
    final maskPtr = netmask.toNativeUtf8();
    final gatewayPtr = gateway.toNativeUtf8();
    final metricPtr = metric.toNativeUtf8();

    try {
      _bindings.add_route(
        destPtr as Pointer<Char>,
        maskPtr as Pointer<Char>,
        gatewayPtr as Pointer<Char>,
        metricPtr as Pointer<Char>,
      );
      print('[EasyNetwork] Route added successfully');
    } catch (e) {
      print('[EasyNetwork] Error adding route: $e');
      rethrow;
    } finally {
      malloc.free(destPtr);
      malloc.free(maskPtr);
      malloc.free(gatewayPtr);
      malloc.free(metricPtr);
    }
  }

  Future<void> cleanRoute() async {
    print('[EasyNetwork] Cleaning routes');
    try {
      _bindings.clean_route();
      print('[EasyNetwork] Routes cleaned successfully');
    } catch (e) {
      print('[EasyNetwork] Error cleaning routes: $e');
      rethrow;
    }
  }
}
