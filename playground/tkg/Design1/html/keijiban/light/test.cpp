#include <sys/event.h>
#include <unistd.h>

#include <chrono>
#include <iostream>

int main() {
  // イベント数とイベントリストのサイズを設定
  const int numEvents = 1000000;
  const int eventListSize = numEvents * sizeof(struct kevent);

  // kqueueを作成
  int kq = kqueue();
  if (kq == -1) {
    std::cerr << "Failed to create kqueue" << std::endl;
    return 1;
  }

  // イベントリストを作成
  struct kevent* events = new struct kevent[numEvents];

  // まとめて登録する方法の処理時間を計測
  auto start = std::chrono::high_resolution_clock::now();

  // イベント登録をまとめて行う
  kevent(kq, events, numEvents, NULL, 0, nullptr);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Bulk event registration duration: " << duration.count() << " ms" << std::endl;

  // 個別に登録する方法の処理時間を計測
  start = std::chrono::high_resolution_clock::now();
  close(kq);
  kq = kqueue();
  // イベント登録を個別に行う
  for (int i = 0; i < numEvents; ++i) {
    kevent(kq, events, numEvents, NULL, 0, nullptr);
  }

  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Individual event registration duration: " << duration.count() << " ms" << std::endl;

  // メモリを解放し、kqueueを閉じる
  delete[] events;
  close(kq);

  return 0;
}