import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:auto_size_text/auto_size_text.dart';
import 'easy_network_http.dart';
import 'easy_network_ffi.dart';
import 'package:uuid/uuid.dart';
import 'pages/settings_page.dart';
import 'package:get/get.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:system_tray/system_tray.dart';
import 'package:window_manager/window_manager.dart';

Timer startPeriodicTask({
  required Function task,
  Duration interval = const Duration(seconds: 60),
  bool runImmediately = true,
}) {
  if (runImmediately) task();
  return Timer.periodic(interval, (_) => task());
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  final _membersController = StreamController<List<Map<String, dynamic>>>();
  final _onlineStatusController = StreamController<bool>();

  late Stream<List<Map<String, dynamic>>> _membersStream;
  late Stream<bool> _onlineStatusStream;

  late final String _deviceUuid;
  late final String _memberUuid;

  final _textController = TextEditingController();
  Timer? _routeUpdateTimer;
  Timer? _memberUpdateTimer;
  Timer? _memberStatusTimer;
  Timer? _onlineStatusTimer;
  bool _isJoined = false;

  DateTime? _lastMemberUpdateTime;
  int _secondsSinceLastUpdate = 0;

  EasyNetworkFFI ffi = EasyNetworkFFI();

  final SystemTray _systemTray = SystemTray();
  final WindowManager _windowManager = WindowManager.instance;
  final Menu _menu = Menu();

  @override
  void initState() {
    super.initState();
    initSystemTray();

    // 注册依赖
    Get.put(this);
    Get.put(ffi);

    _deviceUuid = _buildDeviceUuid();
    _textController.text = 'default';
    _membersStream = _createMembersStream();
    _onlineStatusStream = _createOnlineStatusStream();

    Timer.periodic(const Duration(seconds: 1), (timer) {
      if(_isJoined) {
        if (mounted && _lastMemberUpdateTime != null) {
          setState(() {
            _secondsSinceLastUpdate = DateTime.now().difference(_lastMemberUpdateTime!).inSeconds;
          });
        }
      } else {
        setState(() {
          _secondsSinceLastUpdate = 0;
        });
      }
    });

    report_device_sysinfo(uuid: _deviceUuid).catchError((e) {
      print('Failed to report system info: $e');
    });
  }

  @override
  void dispose() {
    _memberStatusTimer?.cancel();
    _onlineStatusTimer?.cancel();
    _memberUpdateTimer?.cancel();
    _routeUpdateTimer?.cancel();
    super.dispose();
  }

  Future<void> resetNetwork() async {
    if(_isJoined) {
      SharedPreferences prefs = await SharedPreferences.getInstance();
      bool forceUseReplyServer = prefs.getBool('forceUseReplyServer') ?? false;
      if(forceUseReplyServer) {
        String replyAddress = prefs.getString('replyServerAddress') ?? '';
        int replyPort = prefs.getInt('replyServerPort') ?? 0;
        String ifname = _textController.text;

        ffi.setReplyServer(replyAddress, replyPort);
        ffi.resetNetwork(ifname);
      }
    } else {
      print("Not joined any network");
    }
  }

  String _buildDeviceId() {
    try {
      return Platform.localHostname;
    } catch (e) {
      return "unknown";
    }
  }

  String _buildDeviceUuid() {
    final _uuid = Uuid();
    try {
      return _uuid.v4();
    } catch (e) {
      return "unknown";
    }
  }

  void updateMemberStatus() async {
    if(_isJoined) {
      try {
        String networkId = _textController.text;
        String memberId = _memberUuid;
        final members = await get_network_members(networkId: networkId, memberId: memberId);
        _membersController.add(members);
        _lastMemberUpdateTime = DateTime.now();        
      } catch (e) {
        _membersController.addError(e);
      }
    } else {
      _membersController.add([]);
    }
  }
  
  Stream<List<Map<String, dynamic>>> _createMembersStream() {
    _memberStatusTimer?.cancel();
    _memberStatusTimer = null;
    _memberStatusTimer = startPeriodicTask(task: updateMemberStatus);
    return _membersController.stream;
  }

  void updateOnlineStatus() async {
    try {
      final onlineStatus = await update_online_status(uuid: _deviceUuid);
      _onlineStatusController.add(onlineStatus);
    } catch (e) {
      _onlineStatusController.addError(e);
    }
  }
  
  Stream<bool> _createOnlineStatusStream() {
    _onlineStatusTimer?.cancel();
    _onlineStatusTimer = null;
    _onlineStatusTimer = startPeriodicTask(task: updateOnlineStatus);

    return _onlineStatusController.stream;
  }

  void startUpdateMember() async {
    // 更新加入标志
    setState(() {
      _isJoined = true;
    });

    // 更新一次成员
    updateMemberStatus();
  }

  void stopUpdateMember() async {
    // 设置已离开状态
    setState(() {
      _isJoined = false;
    });
    // 清空成员列表
    _membersController.add([]);
    _secondsSinceLastUpdate = 0;
  }

  void setReplyServerAddress(Map<String, dynamic> result) async {
    final servers = result['server_info'];
    var replyAddress = servers['reply_address'].toString();
    try {
      if (replyAddress.startsWith('127.')) {
        replyAddress = 'localhost';
      } else {
        final addresses = await InternetAddress.lookup(replyAddress);
        replyAddress = addresses.first.address;
        print('Resolved IP: $replyAddress');
      }
    } catch (e) {
      print('Failed to resolve address: $e');
    }
    final replyPort = int.tryParse(servers['reply_port'].toString()) ?? 0;
    await ffi.setReplyServer(replyAddress, replyPort);
  }

  void startNetwork(Map<String, dynamic> result) async {
    final member_info = result['member_info'];
    final dns_info = result['dns_info'];

    final ifname = member_info['name'].toString();
    final ifdesc = member_info['desc'].toString();
    final ip = member_info['ipv4_address'].toString();
    final netmask = member_info['subnet_mask'].toString();
    final mtu = member_info['mtu'].toInt();
    
    final domain = dns_info['domain'].toString();
    final nameServer = dns_info['name_server'].toString();
    final searchList = dns_info['search_list'].toString();
    await ffi.joinNetwork(ifname, ifdesc, ip, netmask, mtu, domain, nameServer, searchList);
  }

  void updateRoute(Map<String, dynamic> result) async {
    final routes = result['route_info'];
    for (final route in routes) {
      final destination = route['dest'].toString();
      final netmask = route['netmask'].toString();
      final gateway = route['gateway'].toString();
      final metric = route['metric'].toString();
      await ffi.addRoute(
          destination: destination,
          netmask: netmask,
          gateway: gateway,
          metric: metric);
    }
    print('Updated routes successfully');
  }

  Widget _buildNetworkIdSection() {
    return Column(
      mainAxisAlignment: MainAxisAlignment.start,
      children: [
        const Text(
          'Enter Network ID to join a new network.',
          style: TextStyle(
            fontSize: 16,
            fontWeight: FontWeight.bold,
            color: Colors.grey,
          ),
        ),
        const SizedBox(height: 10),
        TextField(
          controller: _textController,
          decoration: const InputDecoration(
            labelText: 'NETWORK ID',
            border: OutlineInputBorder(),
          ),
        ),
        const SizedBox(height: 10),
        Row(
          mainAxisAlignment: MainAxisAlignment.end,
          children: [
            ElevatedButton(
              onPressed: !_isJoined
                  ? () async {
                      print('Joining network...');
                      try {
                        final result = await joinNetwork(
                          uuid: _deviceUuid,
                          networkId: _textController.text,
                        );
                        _memberUuid = result['member_info']['id'].toString();
                        print('Joined network successfully: $result');
                        // 设置服务器
                        setReplyServerAddress(result);
                        // 启动网络
                        startNetwork(result);
                        // 更新路由
                        Future.delayed(const Duration(seconds: 5), () {
                          updateRoute(result);
                        });
                        // 定时更新成员信息
                        startUpdateMember();
                      } catch (e) {
                        print('Failed to join network: $e');
                      }
                    }
                  : null,
              child: Text('Join'),
              style: ElevatedButton.styleFrom(
                backgroundColor: _isJoined ? Colors.grey : null, // 使用新标志位
              ),
            ),
            SizedBox(width: 10),
            ElevatedButton(
              onPressed: _isJoined
                  ? () async {
                      try {
                        await leaveNetwork(
                          deviceId: _buildDeviceId(),
                          networkName: _textController.text,
                        );
                        await ffi.leaveNetwork(_textController.text);

                        // 停止定时器
                        _memberUpdateTimer?.cancel();
                        _memberUpdateTimer = null;

                        _routeUpdateTimer?.cancel();
                        _routeUpdateTimer = null;

                        stopUpdateMember();
                        print('Left network successfully');
                      } catch (e) {
                        print('Failed to leave network: $e');
                      }
                    }
                  : null,
              child: Text('Leave'),
              style: ElevatedButton.styleFrom(
                backgroundColor: !_isJoined ? Colors.grey : null, // 使用新标志位
              ),
            ),
          ],
        )
      ],
    );
  }

  Widget _buildMember() {
    return StreamBuilder<List<Map<String, dynamic>>>(
      stream: _membersStream,
      builder: (context, snapshot) {
        if(!_isJoined) {
          return const Center(
            child: Text(
              'Not joined network',
              style: TextStyle(
                fontSize: 16,
                color: Colors.grey,
              ),
            ),
          );
        }

        if (snapshot.connectionState == ConnectionState.waiting) {
          return const Center(child: CircularProgressIndicator());
        }
        if (snapshot.hasError) {
          return const Center(child: Text('加载成员失败'));
        }
        final members = snapshot.data ?? [];
        if (members.isEmpty) {
          return const Center(
            child: Text(
              '空空如也',
              style: TextStyle(
                fontSize: 16,
                color: Colors.grey,
              ),
            ),
          );
        }

        return Container(
          color: Colors.grey[200],
          padding: const EdgeInsets.all(1.0),
          child: GridView.builder(
            shrinkWrap: false,
            physics: const BouncingScrollPhysics(),
            gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
              crossAxisCount: 4,
              childAspectRatio: 1.0,
              mainAxisSpacing: 10.0,
              crossAxisSpacing: 10.0,
            ),
            itemCount: members.length,
            itemBuilder: (context, index) {
              final member = members[index];
              return Tooltip(
                message: member['status'],
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(
                      member['icon'],
                      size: 32,
                      color: member['online'] ? Colors.green : Colors.grey,
                    ),
                    const SizedBox(height: 8),
                    AutoSizeText(
                      member['name'],
                      style: const TextStyle(
                        fontSize: 12,
                        fontWeight: FontWeight.bold,
                      ),
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      minFontSize: 9,
                    ),
                  ],
                ),
              );
            },
          ),
        );
      },
    );
  }

  Widget _buildMemberSection() {
    return Expanded(
      child: Column(
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Align(
                alignment: Alignment.centerLeft,
                child: Text(
                  'Network Members',
                  style: TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
              Row(
                children: [
                  if(_isJoined)
                    Text(
                      '${_secondsSinceLastUpdate}s before updated',
                      style: const TextStyle(fontSize: 12),
                    ),
                  IconButton(
                    icon: Icon(Icons.refresh,
                      color: _isJoined ? null : Colors.grey,
                      ),
                    onPressed: _isJoined ? () async {
                      updateMemberStatus();
                    } : null,
                  ),
                ],
              ),
            ],
          ),
          Expanded(
            child: _buildMember(),
          ),
        ],
      ),
    );
  }

  Widget _buildOnlineStatusSection() {
    return StreamBuilder<bool>(
        stream: _onlineStatusStream,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Row(
              mainAxisAlignment: MainAxisAlignment.start,
              children: [
                Icon(Icons.circle, size: 12, color: Colors.grey),
                Text(' 加载中...'),
              ],
            );
          }
          if (snapshot.hasError) {
            return const Row(
              mainAxisAlignment: MainAxisAlignment.start,
              children: [
                Icon(Icons.circle, size: 12, color: Colors.red),
                Text(' 未连接至服务器'),
              ],
            );
          }

          final isOnline = snapshot.data ?? false;
          return Row(
            mainAxisAlignment: MainAxisAlignment.start,
            children: [
              Icon(Icons.circle,
                  size: 12, color: isOnline ? Colors.green : Colors.grey),
              Text(isOnline ? ' 在线' : ' 离线'),
            ],
          );
        });
  }

  Widget _buildOnlineStatusSection2() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        _buildOnlineStatusSection(),
        IconButton(
          constraints: BoxConstraints(
            minWidth: 15,
            minHeight: 15,
          ),
          icon: Icon(Icons.refresh, size: 20, color: Colors.green),
          onPressed: () {
            updateOnlineStatus();
          },
        ),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
        actions: [
          IconButton(
            icon: Icon(Icons.settings),
            onPressed: () async {
              Navigator.push(
                context,
                MaterialPageRoute(builder: (context) => SettingsPage()),
              ).then((value) async {
                await resetNetwork();
              });
            },
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(10.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          children: <Widget>[
            const SizedBox(height: 30),
            _buildNetworkIdSection(),
            const Divider(),
            _buildMemberSection(),
            const Divider(),
            _buildOnlineStatusSection2()
          ],
        ),
      ),
    );
  }

  Future<void> initSystemTray() async {
    // 加载托盘图标
    const String iconPath = 'assets/logo.ico';
    
    // 创建菜单项
    await _menu.buildFrom([
      MenuItemLabel(
        label: '显示窗口',
        onClicked: (meunItem) => _windowManager.show(),
      ),
      MenuItemLabel(
        label: '隐藏窗口',
        onClicked: (meunItem) => _windowManager.hide(),
      ),
      SubMenu(
        label: '语言',
        children: [
          MenuItemLabel(
            label: '中文',
            onClicked: (menuItem) async {
              debugPrint("中文");
            },
          ),
          MenuItemLabel(
            label: 'English',
            onClicked: (menuItem) async {
              debugPrint("English");
            },
          ),
        ]
      ),
      SubMenu(
        label: '主题',
        children: [
          MenuItemLabel(
            label: 'dark',
            onClicked: (menuItem) async {
              debugPrint("dark");
            },
          ),
          MenuItemLabel(
            label: 'light',
            onClicked: (menuItem) async {
              debugPrint("light");
            },
          ),
        ]
      ),
      MenuSeparator(),
      MenuItemLabel(
        label: '退出',
        onClicked: (meunItem) => _windowManager.destroy(),
      ),
    ]);

    // 等待窗口管理器初始化完成
    await _windowManager.ensureInitialized();

    // 初始化系统托盘
    await _systemTray.initSystemTray(
      title: "EasyNetwork",
      iconPath: iconPath,
    );
    await _systemTray.setContextMenu(_menu);
    
    // 设置托盘菜单
    _systemTray.registerSystemTrayEventHandler((eventName) {
      if (eventName == kSystemTrayEventClick) {
        _windowManager.isVisible().then((visible) {
          if (visible) {
            _windowManager.hide();
          } else {
            _windowManager.show();
            _windowManager.focus();
          }
        });
      } else if (eventName == kSystemTrayEventRightClick) {
        _systemTray.popUpContextMenu();
      }
    });
  }
}
