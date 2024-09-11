# OpenWrt-XDP-Sample

将 [libbpf-bootstrap](https://github.com/libbpf/libbpf-bootstrap) 的 [XDP Example](https://github.com/libbpf/libbpf-bootstrap?tab=readme-ov-file#xdp) 移植到 OpenWrt（**树莓派4B 64bits**）中。

## 构建 OpenWrt 系统
> 采用[源码构建](https://openwrt.org/docs/guide-developer/toolchain/use-buildsystem)的目的是为了开启 [BTF](https://www.kernel.org/doc/html/latest/bpf/btf.html) 相关的内核配置。

```bash
# 获取 OpenWrt 源码
git clone https://git.openwrt.org/openwrt/openwrt.git

# 切换到当前最新的 v23.05.4 版本
cd openwrt
git checkout v23.05.4
make distclean

# 更新 feeds
./scripts/feeds update -a
./scripts/feeds install -a

# 更新配置文件，打开 BTF 相关的配置项
make menuconfig

# 构建版本（耗时较长）
make -j$(nproc) defconfig download clean world
```

更新后的配置文件，可以查看 [.config](./.config)。

编译成功后，会在 `openwrt/bin/targets/bcm27xx/bcm2711` 目录下生成版本文件，将其烧录到树莓派中。

烧录成功后，登陆系统，通过以下命令生成所需的 [vmlinux.h](https://github.com/iovisor/bcc/tree/master/libbpf-tools#vmlinuxh-generation) 文件：

```bash
# 生成对应内核版本的 `vmlinux.h` 文件
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
```

## 编译 OpenWrt ipk 包

将 `mypackages` 注册到 openwrt 构建系统中，并更新 feeds：

```bash
./scripts/feeds update mypackages
./scripts/feeds install -a -p mypackages
```

执行 `make menuconfig` 勾选 `Examples -> xdppass` 包。

执行 `make package/xdppass/{clean,compile} V=s` 命令构建 `xdppass` 包，位置在 `openwrt/bin/packages/aarch64_cortex-a72/mypackages/hellobpf_1.0-1_aarch64_cortex-a72.ipk`。

拷贝到 OpenWrt 系统中，可以通过 `opkg` 安装。

执行 `xdppass br-lan`（br-lan 为指定的网口）：

```bash
root@OpenWrt:~# xdppass br-lan
libbpf: loading object 'xdppass_bpf' from buffer
libbpf: elf: section(2) .symtab, size 120, link 1, flags 0, type=2
libbpf: elf: section(3) xdp, size 120, link 0, flags 6, type=1
libbpf: sec 'xdp': found program 'xdp_pass' at insn offset 0 (0 bytes), code size 15 insns (120 bytes)
libbpf: elf: section(4) .rodata.str1.1, size 16, link 0, flags 32, type=1
libbpf: elf: section(5) license, size 4, link 0, flags 3, type=1
libbpf: license of xdppass_bpf is GPL
libbpf: elf: section(6) .BTF, size 734, link 0, flags 0, type=1
libbpf: elf: section(7) .BTF.ext, size 220, link 0, flags 0, type=1
libbpf: looking for externs among 5 symbols...
libbpf: collected 0 externs total
libbpf: map '.rodata.str1.1' (global data): at sec_idx 4, offset 0, flags 80.
libbpf: map 0 is ".rodata.str1.1"
libbpf: loading kernel BTF '/sys/kernel/btf/vmlinux': 0
libbpf: map '.rodata.str1.1': created successfully, fd=4
libbpf: sec 'xdp': found 2 CO-RE relocations
libbpf: CO-RE relocating [2] struct xdp_md: found target candidate [8456] struct xdp_md in [vmlinux]
libbpf: prog 'xdp_pass': relo #0: <byte_off> [2] struct xdp_md.data_end (0:1 @ offset 4)
libbpf: prog 'xdp_pass': relo #0: matching candidate #0 <byte_off> [8456] struct xdp_md.data_end (0:1 @ offset 4)
libbpf: prog 'xdp_pass': relo #0: patched insn #0 (LDX/ST/STX) off 4 -> 4
libbpf: prog 'xdp_pass': relo #1: <byte_off> [2] struct xdp_md.data (0:0 @ offset 0)
libbpf: prog 'xdp_pass': relo #1: matching candidate #0 <byte_off> [8456] struct xdp_md.data (0:0 @ offset 0)
libbpf: prog 'xdp_pass': relo #1: patched insn #1 (LDX/ST/STX) off 0 -> 0
Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` to see output of the BPF programs.
```
