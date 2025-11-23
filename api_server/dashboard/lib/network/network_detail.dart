import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'network_member.dart';
import '../network_editor/network_editor.dart';
import 'network_info.dart';
import 'network_http.dart';
import 'certificate_manager.dart'; // 导入证书管理文件

class NetworkDetail extends StatefulWidget {
  final NetworkInfo network;
  final VoidCallback onDeleteNetwork;
  final VoidCallback onClose;

  const NetworkDetail({
    required this.network,
    required this.onDeleteNetwork,
    required this.onClose,
  });

  @override
  State<NetworkDetail> createState() => _NetworkDetailState();
}

class _NetworkDetailState extends State<NetworkDetail> {
  int selectedIndex = 0;

  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Material(
        color: Colors.white,
        child: Column(
          children: [
            _buildTitleSection(),
            const Divider(height: 1),
            _buildNetworkSection(),
            const Divider(height: 1),
            _buildDeleteNetworkSection(),
          ],
        ),
      );
  }

  // 标题栏
  Widget _buildTitleSection() {
    return Container(
      padding: const EdgeInsets.all(20),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          IconButton(
            icon: const Icon(Icons.arrow_back),
            onPressed: widget.onClose,
          ),
          const SizedBox(width: 20),
          Expanded(
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Network ID',
                      style: TextStyle(fontSize: 16, color: Colors.grey),
                    ),
                    Row(
                      children: [
                        Text(
                          widget.network.basic.id,
                          style: const TextStyle(fontSize: 18),
                        ),
                        const SizedBox(width: 8),
                        IconButton(
                          icon: const Icon(Icons.content_copy, size: 16),
                          onPressed: () {
                            Clipboard.setData(ClipboardData(text: widget.network.basic.id));
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(
                                content: Text('Network ID copied to clipboard'),
                                duration: Duration(seconds: 2),
                              ),
                            );
                          },
                          tooltip: 'Copy Network ID',
                          padding: EdgeInsets.zero,
                          constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
                        ),
                      ],
                    ),
                  ],
                ),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Network Name',
                      style: TextStyle(fontSize: 16, color: Colors.grey),
                    ),
                    Row(
                      children: [
                        Text(
                          widget.network.basic.name,
                          style: const TextStyle(fontSize: 24),
                        ),
                        const SizedBox(width: 8),
                        IconButton(
                          icon: const Icon(Icons.content_copy, size: 16),
                          onPressed: () {
                            Clipboard.setData(ClipboardData(text: widget.network.basic.name));
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(
                                content: Text('Network Name copied to clipboard'),
                                duration: Duration(seconds: 2),
                              ),
                            );
                          },
                          tooltip: 'Copy Network Name',
                          padding: EdgeInsets.zero,
                          constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
                        ),
                      ],
                    ),
                  ],
                ),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'Devices',
                      style: TextStyle(fontSize: 16, color: Colors.grey),
                    ),
                    Text(
                      widget.network.basic.devices,
                      style: const TextStyle(fontSize: 18),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildNetworkLeftSection() {
    return Container(
      child: NavigationRail(
        selectedIndex: selectedIndex,
        onDestinationSelected: (index) {
          setState(() {
            selectedIndex = index;
          });
        },
        labelType: NavigationRailLabelType.all,
        destinations: const [
          NavigationRailDestination(
            icon: Icon(Icons.people),
            label: Text('Members'),
          ),
          NavigationRailDestination(
            icon: Icon(Icons.settings),
            label: Text('Settings'),
          ),
          NavigationRailDestination(
            icon: Icon(Icons.security),
            label: Text('Certificates'),
          ),
        ],
      )
    );
  }

  Widget _buildNetworkRightSection() {
    return Container(
      child: Expanded(
        child: IndexedStack(
          index: selectedIndex,
          children: [
            // Members Content
            _buildMembers(),
            // Settings Content
            SingleChildScrollView(
              child: Padding(
                padding: const EdgeInsets.all(20),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    _buildNetworkSettings(),
                  ],
                ),
              ),
            ),
            // Certificates Content - 使用新的证书管理组件
            CertificateManager(
              networkId: widget.network.basic.id,
              networkName: widget.network.basic.name,
            ),
          ],
        ),
      ),
    );
  }

  // 网络内容编辑
  Widget _buildNetworkSection() {
    return Container(
      child: Expanded(
        child: Row(
          children: [
            _buildNetworkLeftSection(),
            const VerticalDivider(width: 1),
            _buildNetworkRightSection(),
          ],
        ),
      ),
    );
  }

  // 删除网络
  Widget _buildDeleteNetworkSection() {
    return Container(
      padding: const EdgeInsets.all(20),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          ElevatedButton(
            onPressed: widget.onDeleteNetwork,
            style: ElevatedButton.styleFrom(
              backgroundColor: const Color.fromRGBO(255, 204, 142, 1),
            ),
            child: const Text('Delete Network'),
          ),
        ],
      ),
    );
  }

  // 网络成员
  Widget _buildMembers() {
    return NetworkMember(networkId: widget.network.basic.id);
  }

  // 网络设置
  Widget _buildNetworkSettings() {
    return NetworkEditor(
      network: widget.network,
    );
  }
}