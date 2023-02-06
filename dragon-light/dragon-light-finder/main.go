package main

import (
	"fmt"
	"net"
)

func main() {
	fmt.Println("Scanning for Dragon Light...")

	pc, err := net.ListenPacket("udp4", ":8829")
	if err != nil {
		panic(err)
	}
	defer pc.Close()

	addr, err := net.ResolveUDPAddr("udp4", "255.255.255.255:65534")
	if err != nil {
		panic(err)
	}

	for true {
		_, err = pc.WriteTo([]byte("darkdragons"), addr)
		if err != nil {
			panic(err)
		}

		buf := make([]byte, 64)

		n, remote, err := pc.ReadFrom(buf)
		if err != nil {
			panic(err)
		}

		s := string(buf[:n])
		remoteIP := remote.(*net.UDPAddr).IP.String()

		if s == "dragonlight" {
			fmt.Println()
			for i := 0; i < 10; i++ {
				fmt.Println()
			}
			fmt.Printf("Found Dragon Light at http://%s\n", remoteIP)
			break
		}

		fmt.Print(".")
	}
}
