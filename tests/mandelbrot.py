import dpp
import shutil

max_iter = 100

def mandelbrot(cr, ci):
    zr = cr
    zi = ci

    j = 0
    while j < max_iter:
        zr2 = zr * zr
        zi2 = zi * zi

        if zr2 + zi2 <= 4:
            zi = dpp.fma(zr + zr, zi,  ci)
            zr = zr2 - zi2 + cr
        else:
            break

        j += 1

    return j

def main():
    w, h = shutil.get_terminal_size()
    h -= 1

    x0 = dpp.d32(-2)
    y = dpp.d32(1.15)
    x1 = dpp.d32(1)
    y1 = dpp.d32(-1.15)

    dx = (x1 - x0) / w
    dy = (y1 - y) / h

    x0 += dx / 2
    y += dy / 2

    for i in range(h):
        x = x0
        for j in range(w):
            t = mandelbrot(x, y) / max_iter
            olt = 1 - t

            r = int(9 * 255 * (olt * t * t * t))
            g = int(15 * 255 * (olt * olt * t * t))
            b = int(8.5 * 255 * (olt * olt * olt * t))

            print(f"\033[48;2;{r};{g};{b}m ", end="")

            x += dx

        y += dy

    print("\033[0m", end="")

if __name__ == "__main__":
    main()
