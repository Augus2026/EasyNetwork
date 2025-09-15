import 'package:flutter/material.dart';
import '../network/network_info.dart';

// dhcp设置
class Ipv4AutoAssignNetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const Ipv4AutoAssignNetworkEditor({super.key, required this.network});

  @override
  State<Ipv4AutoAssignNetworkEditor> createState() => _Ipv4AutoAssignNetworkEditorState();
}

class _Ipv4AutoAssignNetworkEditorState extends State<Ipv4AutoAssignNetworkEditor> {
  String? selectedIP;

  final rangeStartController = TextEditingController();
  final rangeEndController = TextEditingController();

  @override
  void initState() {
    super.initState();

    rangeStartController.addListener(() {
      widget.network.dhcp.allocType = 'advanced';
      widget.network.dhcp.rangeStart = rangeStartController.text;
    });

    rangeEndController.addListener(() {
      widget.network.dhcp.allocType = 'advanced';
      widget.network.dhcp.rangeEnd = rangeEndController.text;
    });

    setState(() {
      selectedIP = widget.network.dhcp.selectedRange;
    });
  }

  @override
  void dispose() {
    rangeStartController.dispose();
    rangeEndController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return DefaultTabController(
      length: 2,
      initialIndex: widget.network.dhcp.allocType == 'easy' ? 0 : 1,
      child: Card(
        margin: const EdgeInsets.only(bottom: 20),
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Text(
                'IPv4 Auto-Assign',
                style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              const TabBar(tabs: [Tab(text: 'Easy'), Tab(text: 'Advanced')]),
              const SizedBox(height: 10),
              SizedBox(
                height: 400,
                child: TabBarView(
                  children: [
                    _buildEasyIpv4AutoAssignSection(),
                    _buildAdvancedIpv4AutoAssignSection(),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  DataCell _buildIpRangeCell(String ip) {
    return DataCell(
      Text(
        ip,
        style: TextStyle(color: selectedIP == ip ? Colors.blue : Colors.black),
      ),
      onTap: () async {
        setState(() {
          selectedIP = ip;
        });
        widget.network.dhcp.allocType = 'easy';
        widget.network.dhcp.selectedRange = ip;
      }
    );
  }

  Widget _buildEasyIpv4AutoAssignSection() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Easy', style: TextStyle(fontSize: 16)),
        const SizedBox(height: 10),
        SingleChildScrollView(
          scrollDirection: Axis.horizontal,
          child: DataTable(
            columns: const [
              DataColumn(label: Text('IP Range')),
              DataColumn(label: Text('IP Range')),
              DataColumn(label: Text('IP Range')),
              DataColumn(label: Text('IP Range')),
            ],
            rows: [
              DataRow(
                cells: [
                  _buildIpRangeCell('192.168.1.0/24'),
                  _buildIpRangeCell('192.168.2.0/24'),
                  _buildIpRangeCell('192.168.3.0/24'),
                  _buildIpRangeCell('192.168.4.0/24'),
                ],
              ),
              DataRow(
                cells: [
                  _buildIpRangeCell('192.168.5.0/24'),
                  _buildIpRangeCell('192.168.6.0/24'),
                  _buildIpRangeCell('192.168.7.0/24'),
                  _buildIpRangeCell('192.168.8.0/24'),
                ],
              ),
              DataRow(
                cells: [
                  _buildIpRangeCell('192.168.9.0/24'),
                  _buildIpRangeCell('192.168.10.0/24'),
                  _buildIpRangeCell('192.168.11.0/24'),
                  _buildIpRangeCell('192.168.12.0/24'),
                ],
              ),
              DataRow(
                cells: [
                  _buildIpRangeCell('192.168.13.0/24'),
                  _buildIpRangeCell('192.168.14.0/24'),
                  _buildIpRangeCell('192.168.15.0/24'),
                  _buildIpRangeCell('192.168.16.0/24'),
                ],
              ),
              DataRow(
                cells: [
                  _buildIpRangeCell('192.168.17.0/24'),
                  _buildIpRangeCell('192.168.18.0/24'),
                  _buildIpRangeCell('192.168.19.0/24'),
                  _buildIpRangeCell('192.168.20.0/24'),
                ],
              ),
              DataRow(
                cells: [
                  _buildIpRangeCell('192.168.21.0/24'),
                  _buildIpRangeCell('192.168.22.0/24'),
                  _buildIpRangeCell('192.168.23.0/24'),
                  _buildIpRangeCell('192.168.24.0/24'),
                ],
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildAdvancedIpv4AutoAssignSection() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Advanced', style: TextStyle(fontSize: 16)),
        const SizedBox(height: 20),
        const Text('Add Ipv4 Address Pools', style: TextStyle(fontSize: 16)),
        TextField(
          controller: rangeStartController,
          decoration: InputDecoration(
            labelText: 'Range Start',
            hintText: widget.network.dhcp.rangeStart,
            floatingLabelBehavior: FloatingLabelBehavior.always,
          ),
        ),
        TextField(
          controller: rangeEndController,
          decoration: InputDecoration(
            labelText: 'Range End',
            hintText: widget.network.dhcp.rangeEnd,
            floatingLabelBehavior: FloatingLabelBehavior.always,
          ),
        ),
      ],
    );
  }
}