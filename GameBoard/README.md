# Bảng trò chơi trên Arduino ESP32

## A. Giới thiệu

- Bảng trò chơi được thiết kế với một menu, màn hình bắt đầu, màn chơi chính và màn hình khi kết thúc game.
- Có 4 game là: Flappy Bird, Catch the Fruits, Racing Car và Dinosaur Run.
- Để chơi game, người chơi làm theo hướng dẫn ở mục B.

## Dependencies

- [ThingPulse OLED SSD1306](https://github.com/ThingPulse/esp8266-oled-ssd1306.git): Thư viện màn hình OLED SSD1306 cho ESP8266 và ESP32.
- [Preferences](https://github.com/vshymanskyy/Preferences): Thư viện lưu trữ điểm cao trong hệ thống tệp flash nội bộ của ESP32.

## Cài đặt

1. Clone repository:

```bash
git clone https://github.com/dang1407/ESP32_GAME.git
```

1. Checkout sang nhánh **menu_game**
1. Mở dự án trong Arduino IDE.

1. Cài đặt các thư viện cần thiết bằng Trình quản lý thư viện Arduino.

1. Cấu hình dự án bằng cách chọn bo mạch và cổng thích hợp.

1. Nạp code vào ESP32 của bạn.

## B. Hướng dẫn chơi

### Video giới thiệu

[YouTube: Demo game ESP32](https://youtu.be/vn7H8XjG3O8)

### 1. Hướng dẫn chung

- Game thủ sử dụng 2 nút (trái và phải) để chọn game và chơi.
- Trong menu, nút bên trái dùng để chuyển sang trò chơi tiếp theo và nút bên phải để chọn trò chơi. Game đang được chọn là game có tên to hơn các game khác.
- Trong màn hình bắt đầu bạn nhấn nút bên trái để bắt đầu trò chơi hoặc nhấn nút bên phải để quay lại menu.
- Trong màn hình chơi chính, nút bên trái dùng để di chuyển nhân vật sang trái hoặc lên và nút bên phải thì ngược lại.
- Tốc độ của nhân vật được tăng lên mỗi khi số điểm chia hết cho 10.
- Điểm của người chơi sẽ được tổng kết ở màn hình kết thúc. Ở đây bạn có thể bấm nút bên phải để quay lại màn hình bắt đầu.
- **Hướng dẫn điều khiển chung:**

  1.  Bật ESP32 của bạn với game mà bạn muốn chơi.

  2.  Điều khiển nhân vật bằng các nút hoặc cảm biến được chỉ định.

  3.  Cố gắng di chuyển và ghi càng nhiều điểm càng tốt.

  4.  Cạnh tranh với bạn bè và đồng nghiệp để đạt điểm cao nhất!

### 2. Flappy Bird

- Menu
  ![Hình minh họa](./images/FlappyBirdStart.jpg)
- Trong game Flappy Bird, cách chơi giống hệt game nguyên tác của tác giả Dong Nguyen, bạn nhấn nút trái để chú chim bay lên vượt chướng ngại vật.
- Màn chơi chính:
  ![](./images/FlappyBird.jpg)
- Mỗi khi vượt qua một chướng ngại vật, người chơi sẽ đạt được một điểm.

### 3. Racing Car

- Màn hình bắt đầu

  ![](./images/RacingCarStart.jpg)

- Trong game Racing Car, bạn lái xe của mình trên con đường có 3 làn và phải né tránh các xe đi ngược chiều.
  ![](./images/RacingCarPlay.jpg)
- Đôi lúc sẽ có cả 3 xe đi cùng nhau ở 3 làn đường, bạn không thể né tránh và phải sử dụng đạn để tiêu diệt xe ở làn nào đó để đi qua.
- Để bắn đạn thì có 2 cách:
  - Cách 1: Bấm cùng lúc 2 nút bấm, việc này có tỉ lệ cao khiến ô tô của bạn dịch sang hàng khác sau đó quay lại rồi mới bắn đạn. Bạn cần cân nhắc khi sử dụng tính năng này.
  - Cách 2: Bấm vào chân D4 trên ESP32 để bắn đạn nhưng cảm biến đôi lúc có thể không nhạy bén nên bạn cần bấm nhiều lần.
- Mỗi khi vượt qua và có một chiếc ô tô ra khỏi màn chơi, người chơi sẽ được cộng một điểm.

### 4. Catch the Fruits

- Menu
  ![](./images/CatchTheFruitStart.jpg)
- Có thể nói tựa game Catch the Fruits là tựa game dễ nhất, giải trí nhất trong những game mà chúng tôi có.
- Bạn sử dụng nút bấm để di chuyển giỏ hoa quả, nút bấm bên trái để di chuyển sang trái, nút bên phải để di chuyển sang phải, hứng càng nhiều hoa quả càng tốt đồng thời cần né tránh các viên đá.
  ![](./images/CatchTheFruitPlay.jpg)
- Mỗi khi hứng được một hoa quả người chơi sẽ được cộng một điểm, khi chạm vào viên đá thì trò chơi sẽ kết thúc.

### 5. Dinosaur Run

- Menu
  ![](./images/DinosaurRunStart.jpg)
- Tựa game Dinosaur Run có kịch bản tương tự với game xuất hiện trên trình duyệt Chrome khi không có Internet (thường được gọi với cái tên thân thương "Khủng long mất mạng").
  ![](./images/DinosaurRunPlay.jpg)
- Người chơi sử dụng nút bấm bên trái để làm chú khủng long nhảy lên.

## C. Yêu cầu phần cứng

- 1 ESP32 Dev kit
- 1 Màn hình Oled SSD1306 tương thích với ESP32.
- 2 nút bấm
- 1 còi
- 1 Pin 411136 3.7 V và 1 mạch sạc.
- Dây dẫn
- 1 Bảng mạch PCB đục lỗ 5 x 7cm
