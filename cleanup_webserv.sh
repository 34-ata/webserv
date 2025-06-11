#!/bin/bash

echo -e "\n\033[1;33mCleaning up test-created files...\033[0m"

# Yüklenen tüm dosyalar
if [ -d "www/uploads" ]; then
	find www/uploads -type f -name "upload_*" -exec rm -f {} \;
	echo "Cleaned uploaded test files from www/uploads/"
fi

# Sadece testte oluşturulan silinebilir dosya
if [ -f "www/images/deletable.txt" ]; then
	rm -f "www/images/deletable.txt"
	echo "Removed: www/images/deletable.txt"
fi

# CGI test dosyası
if [ -f "www/cgi-bin/script.py" ]; then
	rm -f "www/cgi-bin/script.py"
	echo "Removed: www/cgi-bin/script.py"
fi

# www/uploads veya www/cgi-bin boşsa silme, sadece dosyaları kaldır
# Dizin yapısını koruyoruz

echo -e "\n\033[1;32mCleanup complete.\033[0m"
