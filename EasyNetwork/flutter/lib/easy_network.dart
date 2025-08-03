import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:auto_size_text/auto_size_text.dart';
import 'easy_network_http.dart';
import 'easy_network_ffi.dart';
import 'package:uuid/uuid.dart';
import 'pages/settings_page.dart';

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
  final _textController = TextEditingController();
  Timer? _routeUpdateTimer;
  Timer? _memberUpdateTimer;
  Timer? _memberStatusTimer;
  Timer? _onlineStatusTimer;
  bool _isJoined = false;

  DateTime? _lastMemberUpdateTime;
  int _secondsSinceLastUpdate = 0;

  @override
  void initState() {
    super.initState();
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

    report_sysinfo(uuid: _deviceUuid).catchError((e) {
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
        final members =
            await get_network_members(networkId: _textController.text);
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
                          deviceId: _buildDeviceId(),
                          networkName: _textController.text,
                        );
                        print('Joined network successfully: $result');

                        final ffi = EasyNetworkFFI();
                        // 设置服务器
                        final servers = result['server'];

                        var replyAddress = servers['reply_address'].toString();
                        try {
                          final addresses = await InternetAddress.lookup(replyAddress);
                          replyAddress = addresses.first.address;
                          print('Resolved IP: $replyAddress');
                        } catch (e) {
                          print('Failed to resolve address: $e');
                        }

                        final replyPort = servers['reply_port'] is int
                            ? servers['reply_port'] as int
                            : int.parse(servers['reply_port'].toString());

                        await ffi.setServer(replyAddress, replyPort);

                        // 启动网络
                        final interface = result['interface'];
                        final ifname = interface['name'].toString();
                        final ifdesc = interface['desc'].toString();
                        final ip = interface['ipv4_address'].toString();
                        final netmask = interface['subnet_mask'].toString();
                        final mtu = interface['mtu'].toString();
                        final domain = interface['domain'].toString();
                        final nameServer = interface['name_server'].toString();
                        final searchList = interface['search_list'].toString();
                        await ffi.joinNetwork(ifname, ifdesc, ip, netmask, mtu,
                            domain, nameServer, searchList);

                        // 更新成员
                        updateMember() async {
                          try {
                            await update_member_online_status(
                                deviceId: _buildDeviceId(),
                                networkName: _textController.text);
                          } catch (e) {
                            print('Failed to update member: $e');
                          }
                        }
                        _memberUpdateTimer = startPeriodicTask(task: updateMember);

                        // 启动路由更新定时器
                        updateRoute() async {
                          try {
                            final routes = await update_route(
                                deviceId: _buildDeviceId(),
                                networkName: _textController.text);
                            print('routes: $routes');

                            final ffi = EasyNetworkFFI();
                            await ffi.cleanRoute();
                            print('Cleaned routes successfully');

                            for (final route in routes) {
                              final destination =
                                  route['destination'].toString();
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
                          } catch (e) {
                            print('Failed to update routes: $e');
                          }
                        }
                        _routeUpdateTimer = startPeriodicTask(task: updateRoute);

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
                        final ffi = EasyNetworkFFI();
                        await ffi.leaveNetwork();

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
              return Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Icon(
                    member['icon'],
                    size: 32,
                    color: member['status'] ? Colors.green : Colors.grey,
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
            onPressed: () {
              Navigator.push(
                context,
                MaterialPageRoute(builder: (context) => SettingsPage()),
              );
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
}
