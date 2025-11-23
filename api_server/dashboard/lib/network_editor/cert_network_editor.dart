import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:convert';
import 'package:http/http.dart' as http;
import '../network/network_info.dart';
import '../web.dart';

// 证书文件数据类
class CertificateFile {
  final String filename;
  final String description;

  CertificateFile(this.filename, this.description);
}

// 证书配置组件
class CertNetworkEditor extends StatefulWidget {
  final NetworkInfo network;

  const CertNetworkEditor({
    Key? key,
    required this.network,
  }) : super(key: key);

  @override
  _CertNetworkEditorState createState() => _CertNetworkEditorState();
}

class _CertNetworkEditorState extends State<CertNetworkEditor> {
  final TextEditingController _caCertController = TextEditingController();
  final TextEditingController _caKeyController = TextEditingController();
  final TextEditingController _serverCertController = TextEditingController();
  final TextEditingController _serverKeyController = TextEditingController();

  @override
  void dispose() {
    _caCertController.dispose();
    _caKeyController.dispose();
    _serverCertController.dispose();
    _serverKeyController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [          
        // CA证书配置
        _buildCertificateCard(
          title: 'CA Certificate',
          icon: Icons.security,
          files: [
            CertificateFile('ca-cert.pem', 'CA Root Certificate'),
            CertificateFile('ca-key.pem', 'CA Private Key'),
          ],
          controllers: [_caCertController, _caKeyController],
        ),
        
        const SizedBox(height: 20),
        
        // 服务器证书配置
        _buildCertificateCard(
          title: 'Server Certificate',
          icon: Icons.cloud,
          files: [
            CertificateFile('server-cert.pem', 'Server Certificate'),
            CertificateFile('server-key.pem', 'Server Private Key'),
          ],
          controllers: [_serverCertController, _serverKeyController],
        ),
      ],
    );
  }

  Widget _buildCertificateCard({
    required String title,
    required IconData icon,
    required List<CertificateFile> files,
    required List<TextEditingController> controllers,
  }) {
    return Card(
      elevation: 2,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(icon, color: Colors.blue),
                const SizedBox(width: 8),
                Text(
                  title,
                  style: const TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 16),
            ...files.asMap().entries.map((entry) => 
              _buildCertificateFileItem(entry.value, controllers[entry.key])
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildCertificateFileItem(CertificateFile file, TextEditingController controller) {
    return Container(
      margin: const EdgeInsets.only(bottom: 12),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            file.filename,
            style: const TextStyle(
              fontWeight: FontWeight.bold,
              fontSize: 14,
            ),
          ),
          const SizedBox(height: 4),
          Text(
            file.description,
            style: TextStyle(
              fontSize: 12,
              color: Colors.grey.shade600,
            ),
          ),
          const SizedBox(height: 8),
          TextField(
            controller: controller,
            maxLines: 6,
            minLines: 3,
            decoration: InputDecoration(
              hintText: 'Paste ${file.filename} content here...',
              border: OutlineInputBorder(),
              contentPadding: const EdgeInsets.all(12),
            ),
          ),
        ],
      ),
    );
  }

}