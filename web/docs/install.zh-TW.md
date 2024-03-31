# 安裝

我們一共有兩種方法可以得到執行檔。如果你是使用者，你不需要自行編譯程式，我們建議你可以參考已編譯版本，見 [已編譯版本](#已編譯版本 (推薦))；如果你是開發者，請見 [自行編譯](#自行編譯)。

---

## 已編譯版本 (推薦)

[https://github.com/JueXiuHuang/MapArtist/releases/latest](https://github.com/JueXiuHuang/MapArtist/releases/latest)

從 Github Release 下載壓縮檔並解壓縮即可。

![release](site:images/release.png)

---

## 自行編譯

### 準備工作

MapArtist 支援兩種平台：Windows 和 Linux。

#### Windows

- [Git](https://git-scm.com/): 從 Github 下載專案。
- [MSVC](https://visualstudio.microsoft.com/downloads/) (Microsoft Visual C++): 編譯所需的編譯器。
- [CMake](https://cmake.org/): 專案的建立系統。

!!! note "CMake 版本需要 >= 3.26"

#### Linux

- [Git](https://git-scm.com/): 從 Github 下載專案。

    ```bash
    sudo apt-get install git
    ```

- [GCC](https://gcc.gnu.org/): 編譯所需的編譯器。

    ```bash
    sudo apt-get install build-essential
    ```

- [CMake](https://cmake.org/): 專案的建立系統。

    ```bash
    sudo apt-get install cmake
    ```

    !!! note "CMake 版本需要 >= 3.26"

- [Zlib](https://www.zlib.net/)

    ```bash
    sudo apt-get install zlib1g-dev
    ```

- [OpenSSL](https://www.openssl.org/)

    ```bash
    sudo apt-get install libssl-dev
    ```

---

### 步驟

1. 下載專案。

    ``` bash
    git clone https://github.com/JueXiuHuang/MapArtist
    ```

2. 產生配置檔案。

    ``` bash
    cd MapArtist
    cmake -B build -S .
    ```

3. 編譯程式。

    ``` bash
    cmake --build build
    ```

    執行檔將會產生在 `MapArtist/bin`
