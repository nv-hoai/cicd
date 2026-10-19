# CI/CD với GitHub Actions

Dự án thực hành thiết lập pipeline **CI/CD** tự động bằng GitHub Actions cho một chương trình C kiểm tra số nguyên tố.

📄 [Báo cáo thực hành chi tiết (REPORT.md)](REPORT.md)

## Pipeline

| Workflow | Kích hoạt | Nội dung |
| --- | --- | --- |
| `Continuous Integration (CI) Quality Gate` | Push / Pull Request vào `main` | Lint (Cppcheck) ➔ Build (GCC) ➔ Unit Test |
| `Automated Production Release (CD)` | Push tag `v*` | Build, đóng gói `.tar.gz`, tạo GitHub Release |

## Chạy cục bộ

```bash
make lint   # cần cppcheck
make build
make test
```

## Phát hành phiên bản

```bash
git tag v1.0.0 -m "Phát hành phiên bản chính thức đầu tiên bản v1.0.0"
git push origin v1.0.0
```

Workflow CD sẽ đính kèm `prime-validator-linux-amd64.tar.gz` vào trang Release.
