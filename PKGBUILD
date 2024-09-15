# Maintainer: André Silva 000plagueinc@gmail.com
pkgname=proxy-checker
pkgver=1.0
pkgrel=1
epoch=
pkgdesc="A small program to check proxy avaliability"
arch=('x86_64')
url="https://github.com/Hunter104/proxy-checker"
license=('unknown')
depends=('curl', 'cjson', 'openmp')
source=("$pkgname-$pkgver.tar.gz"
        "$pkgname-$pkgver.patch")
sha256sums=()
validpgpkeys=()

build() {
	cd "$pkgname-$pkgver"
  cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build build
}

package() {
	cd "$pkgname-$pkgver/build"
	make DESTDIR="$pkgdir" install
}
