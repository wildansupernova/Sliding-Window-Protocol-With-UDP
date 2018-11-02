# Sliding-Window-Protocol-With-UDP

## Anggota Kelompok
Mathias Novianto - 13516021<br>
Rizky Andyno Ramadhan - 13516063<br>
Wildan Dicky Alnatara - 13516012<br>

## Petunjuk Penggunaan Program
1. Compile program dengan menjalankan file Makefile dengan menggunakan command `make Makefile`.
2. Setelah dicompile, terdapat dua file yang dapat dijalankan, yaitu file sendfile dan file recvfile.
3.1. Jalankan file receiver dengan command `./recvfile [nama-file-r] [window-size] [buffer-size] [port]`.
3.2. Jalankan file sender dengan command `./sendfile  [nama-file-s] [window-size] [buffer-size] [ip-tujuan] [port]`.
4. Program sendfile akan mengirimkan file yang tertulis pada `[nama-file-s]` kepada recvfile, dan akan disimpan di folder yang sama dengan recvfile berada dengan nama `[nama-file-r]`.
5. Selesai.

**Note**: Simulasi packet corrupt dapat dilakukan dengan menambahkan argumen corrupt [peluang dalam %].
Contoh : `./sendfile doc.txt 10 100 0.0.0.0 8080 corrupt 50` untuk mengirimkan file *doc.txt* dengan tingkat *packet corrupt* sebesar 50%.

## Cara Kerja Sliding Window
### Sisi Sender
Pada sisi sender, sliding window bekerja dengan dua thread; thread pertama sebagai program utama, dan thread kedua sebagai sub-program untuk menerima ACK dari receiver. Pertama-tama, program utama melakukan pengisian *buffer* hingga penuh dengan fungsi `fillBuffer()` dan mengirimkan frame sesuai dengan ukuran sliding window. Pengiriman frame dilakukan dengan memanggil fungsi `sendFrame` yang mengubah tipe data frame menjadi *array of char* representasi dari frame tersebut, lalu mengirimnya ke receiver. Program akan mengecek apakah ACK sudah masuk secara berkala. Apabila frame paling kiri pada sliding window sudah menerima ACK yang valid (sesuai checksum), maka frame tersebut akan dilepas dari buffer dan sliding window pun bergeser. Tiap frame memiliki *timeout*-nya masing-masing, dan apabila *timeout* suatu frame sudah habis, maka frame dianggap *loss* dan sender akan mengirim ulang frame tersebut.

### Sisi Receiver
Pada sisi receiver, sliding window tidak berbeda jauh dengan sliding window pada sisi sender. Bedanya, kali ini terdapat thread untuk menerima frame dari sender. Ketika receiver menerima frame yang *sequence number*-nya berada di rentang sliding window, maka frame akan dicek validitasnya dan apabila sudah sesuai dengan checksumnya, frame tersebut akan disimpan di buffer dan receiver akan mengirimkan ACK. Setelah itu, dilakukan pengecekan secara berkala untuk melihat apakah frame pada pojok kiri sliding window telah diterima. Apabila sudah, receiver akan menulis ke dalam file eksternal data yang ada di dalam frame tersebut dan frame tersebut langsung dilepas dari buffer. Apabila frame yang diterima tidak sesuai dengan checksumnya, maka receiver akan langsung mengirimkan NAK untuk meminta sender untuk mengirim ulang frame tersebut.

## Pembagian Tugas
Mathias -> Pembuatan Receiver, Pengujian Program, Pembuatan Fungsi Utility <br>
Rizky -> Pembuatan Sender, Implementasi Simulasi Packet Corrupt, Makefile, Sedikit Utility <br>
Wildan -> Pembuatan Sender, Pembuatan Receiver, Pengujian Program, Pembuatan Fungsi Utility <br>
