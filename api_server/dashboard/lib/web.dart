import 'package:flutter/foundation.dart';
import 'package:web/web.dart' as web;

String baseUrl = kIsWeb ? '${web.window.location.origin}' : 'http://localhost:1000';