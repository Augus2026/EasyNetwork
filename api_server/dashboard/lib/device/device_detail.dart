import 'package:flutter/material.dart';
import 'device_info.dart';

class DeviceDetail extends StatefulWidget {
final List<DeviceInfo> deviceInfoList;

  const DeviceDetail({
    required this.deviceInfoList,
  });

  @override
  State<DeviceDetail> createState() => _DeviceDetailState();
}

class _DeviceDetailState extends State<DeviceDetail> {
  String layoutType = 'list';

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('设备详细信息'),
        actions: [
          PopupMenuButton<String>(
            icon: const Icon(Icons.view_module),
            onSelected: (value) {
              setState(() {
                layoutType = value;
              });
            },
            itemBuilder: (BuildContext context) => [
              const PopupMenuItem<String>(
                value: 'list',
                child: Text('列表视图'),
              ),
              const PopupMenuItem<String>(
                value: 'grid',
                child: Text('网格视图'),
              ),
              const PopupMenuItem<String>(
                value: 'wrap',
                child: Text('流式布局'),
              ),
            ],
          ),
        ],
      ),
      body: _buildLayout(),
    );
  }

  Widget _buildLayout() {
    switch (layoutType) {
      case 'grid':
        return _buildGrid();
      case 'wrap':
        return _buildWrap();
      default:
        return _buildList();
    }
  }

  Widget _buildList() {
    return ListView.builder(
      itemCount: widget.deviceInfoList.length,
      itemBuilder: (context, index) {
        final deviceInfo = widget.deviceInfoList[index];
        return Card(
          margin: const EdgeInsets.all(8.0),
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  '设备 ${index + 1}',
                  style: const TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.blue,
                  ),
                ),
                const SizedBox(height: 12),
                DataTable(
                  columns: const [
                    DataColumn(
                      label: Text('属性'),
                      columnWidth: FractionColumnWidth(0.3),
                    ),
                    DataColumn(
                      label: Text('值'),
                      columnWidth: FractionColumnWidth(0.7),
                    ),
                  ],
                  rows: [
                    _buildDataRow('uuid', deviceInfo.uuid),
                    _buildDataRow('计算机名称', deviceInfo.computerName),
                    _buildDataRow('处理器数量', deviceInfo.numProcessor),
                    _buildDataRow('内存大小', '${deviceInfo.memory} MB'),
                    _buildDataRow('产品ID', deviceInfo.productId),
                    _buildDataRow('设备ID', deviceInfo.deviceId),
                    _buildDataRow('用户名', deviceInfo.userName),
                    _buildDataRow('产品名称', deviceInfo.productName),
                    _buildDataRow('版本ID', deviceInfo.editionId),
                    _buildDataRow('显示版本', deviceInfo.displayVersion),
                    _buildDataRow('安装日期', deviceInfo.installDate),
                    _buildDataRow('构建编号', deviceInfo.buildNumber),
                    _buildDataRow('最后更新时间', deviceInfo.lastUpdate),
                  ],
                ),
              ],
            ),
          ),
        );
      },
    );
  }

  Widget _buildGrid() {
    return GridView.builder(
      padding: const EdgeInsets.all(8.0),
      gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
        crossAxisCount: 4, // 每行显示2个设备卡片
        crossAxisSpacing: 6.0, // 水平间距
        mainAxisSpacing: 6.0, // 垂直间距
        childAspectRatio: 1.0, // 宽高比
      ),
      itemCount: widget.deviceInfoList.length,
      itemBuilder: (context, index) {
        final deviceInfo = widget.deviceInfoList[index];
        return Card(
          child: Padding(
            padding: const EdgeInsets.all(8.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  '设备 ${index + 1}',
                  style: const TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: Colors.blue,
                  ),
                ),
                const SizedBox(height: 8),
                Expanded(
                  child: SingleChildScrollView(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        ..._buildWrapInfoRows(deviceInfo),
                      ],
                    ),
                  ),
                ),
              ],
            ),
          ),
        );
      },
    );
  }

  DataRow _buildDataRow(String label, String value) {
    return DataRow(
      cells: [
        DataCell(Text(
          label,
          style: const TextStyle(fontWeight: FontWeight.bold),
        )),
        DataCell(Text(value)),
      ],
    );
  }

  Widget _buildInfoRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 2.0),
      child: RichText(
        text: TextSpan(
          style: const TextStyle(fontSize: 12, color: Colors.black),
          children: [
            TextSpan(
              text: '$label: ',
              style: const TextStyle(fontWeight: FontWeight.bold),
            ),
            TextSpan(text: value),
          ],
        ),
      ),
    );
  }
  
  Widget _buildWrap() {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(12.0),
      child: Wrap(
        spacing: 12.0, // 水平间距
        runSpacing: 12.0, // 垂直间距
        children: widget.deviceInfoList.map((deviceInfo) {
          final index = widget.deviceInfoList.indexOf(deviceInfo);
          return SizedBox(
            width: 350, // 固定每个卡片的宽度
            child: Card(
              child: Padding(
                padding: const EdgeInsets.all(12.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      '设备 ${index + 1}',
                      style: const TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                        color: Colors.blue,
                      ),
                    ),
                    const SizedBox(height: 8),
                    ..._buildWrapInfoRows(deviceInfo),
                  ],
                ),
              ),
            ),
          );
        }).toList(),
      )
    );
  }

  List<Widget> _buildWrapInfoRows(DeviceInfo deviceInfo) {
    return [
      _buildInfoRow('uuid', deviceInfo.uuid),
      _buildInfoRow('计算机名称', deviceInfo.computerName),
      _buildInfoRow('处理器数量', deviceInfo.numProcessor),
      _buildInfoRow('内存大小', '${deviceInfo.memory} MB'),
      _buildInfoRow('产品ID', deviceInfo.productId),
      _buildInfoRow('设备ID', deviceInfo.deviceId),
      _buildInfoRow('用户名', deviceInfo.userName),
      _buildInfoRow('产品名称', deviceInfo.productName),
      _buildInfoRow('版本ID', deviceInfo.editionId),
      _buildInfoRow('显示版本', deviceInfo.displayVersion),
      _buildInfoRow('安装日期', deviceInfo.installDate),
      _buildInfoRow('构建编号', deviceInfo.buildNumber),
      _buildInfoRow('最后更新时间', deviceInfo.lastUpdate),
    ];
  }
}