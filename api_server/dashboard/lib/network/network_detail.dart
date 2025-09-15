import 'package:flutter/material.dart';
import 'network_member.dart';
import '../network_editor/network_editor.dart';
import 'network_info.dart';
import 'network_http.dart';

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
                    Text(
                      widget.network.basic.id,
                      style: const TextStyle(fontSize: 18),
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
                    Text(
                      widget.network.basic.name,
                      style: const TextStyle(fontSize: 24),
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
            icon: Icon(Icons.rule),
            label: Text('Flow Rules'),
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
            // Flow Rules Content
            Container(),
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