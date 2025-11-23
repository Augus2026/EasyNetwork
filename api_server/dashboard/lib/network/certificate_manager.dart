import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:convert';
import 'package:http/http.dart' as http;
import '../web.dart';

// 证书文件数据类
class CertificateFile {
  final String filename;
  final String description;

  CertificateFile(this.filename, this.description);
}

// 证书配置组件
class CertificateManager extends StatefulWidget {
  final String networkId;
  final String networkName;

  const CertificateManager({
    Key? key,
    required this.networkId,
    required this.networkName,
  }) : super(key: key);

  @override
  _CertificateManagerState createState() => _CertificateManagerState();
}

class _CertificateManagerState extends State<CertificateManager> {
  // 证书内容控制器
  final TextEditingController _caCertController = TextEditingController();
  final TextEditingController _caKeyController = TextEditingController();
  final TextEditingController _serverCertController = TextEditingController();
  final TextEditingController _serverKeyController = TextEditingController();

  bool _isLoading = false;
  String? _errorMessage;
  String? _successMessage;

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
    return SingleChildScrollView(
      padding: const EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            'Certificate Configuration',
            style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
          ),
          const SizedBox(height: 20),
          
          // 显示错误消息
          if (_errorMessage != null)
            Container(
              width: double.infinity,
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.red.shade50,
                border: Border.all(color: Colors.red.shade200),
                borderRadius: BorderRadius.circular(8),
              ),
              child: Row(
                children: [
                  Icon(Icons.error, color: Colors.red.shade600),
                  const SizedBox(width: 8),
                  Expanded(
                    child: Text(
                      _errorMessage!,
                      style: TextStyle(color: Colors.red.shade800),
                    ),
                  ),
                  IconButton(
                    icon: const Icon(Icons.close),
                    onPressed: () {
                      setState(() {
                        _errorMessage = null;
                      });
                    },
                  ),
                ],
              ),
            ),
          
          // 显示成功消息
          if (_successMessage != null)
            Container(
              width: double.infinity,
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.green.shade50,
                border: Border.all(color: Colors.green.shade200),
                borderRadius: BorderRadius.circular(8),
              ),
              child: Row(
                children: [
                  Icon(Icons.check_circle, color: Colors.green.shade600),
                  const SizedBox(width: 8),
                  Expanded(
                    child: Text(
                      _successMessage!,
                      style: TextStyle(color: Colors.green.shade800),
                    ),
                  ),
                  IconButton(
                    icon: const Icon(Icons.close),
                    onPressed: () {
                      setState(() {
                        _successMessage = null;
                      });
                    },
                  ),
                ],
              ),
            ),
          
          if (_errorMessage != null || _successMessage != null)
            const SizedBox(height: 20),
          
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
          
          const SizedBox(height: 20),
          
          // 提交按钮
          _buildSubmitButton(),
        ],
      ),
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

  Widget _buildSubmitButton() {
    return Card(
      elevation: 2,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Upload Certificates',
              style: TextStyle(
                fontSize: 18,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 16),
            _isLoading
                ? const Center(child: CircularProgressIndicator())
                : ElevatedButton.icon(
                    onPressed: _uploadCertificates,
                    icon: const Icon(Icons.upload_file),
                    label: const Text('Submit Certificate Data'),
                    style: ElevatedButton.styleFrom(
                      minimumSize: const Size(double.infinity, 50),
                    ),
                  ),
          ],
        ),
      ),
    );
  }

  Future<void> _uploadCertificates() async {
    // 验证输入
    if (_caCertController.text.isEmpty ||
        _caKeyController.text.isEmpty ||
        _serverCertController.text.isEmpty ||
        _serverKeyController.text.isEmpty) {
      setState(() {
        _errorMessage = 'Please fill in all certificate fields';
      });
      return;
    }

    setState(() {
      _isLoading = true;
      _errorMessage = null;
      _successMessage = null;
    });

    try {
      final Map<String, dynamic> certificateData = {
        'network_id': widget.networkId,
        'ca_cert': _caCertController.text,
        'ca_key': _caKeyController.text,
        'server_cert': _serverCertController.text,
        'server_key': _serverKeyController.text,
      };

      final response = await http.post(
        Uri.parse('$baseUrl/api/v1/networks/${widget.networkId}/certificates'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode(certificateData),
      );

      if (response.statusCode == 200) {
        final result = jsonDecode(response.body) as Map<String, dynamic>;
        if (result['status'] == 'success') {
          setState(() {
            _successMessage = 'Certificates uploaded successfully!';
          });
        } else {
          setState(() {
            _errorMessage = result['message'] ?? 'Failed to upload certificates';
          });
        }
      } else {
        setState(() {
          _errorMessage = 'HTTP error: ${response.statusCode}';
        });
      }
    } catch (e) {
      setState(() {
        _errorMessage = 'Error uploading certificates: $e';
      });
    } finally {
      setState(() {
        _isLoading = false;
      });
    }
  }
}