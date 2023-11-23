systemd-run --user --pty --same-dir --wait --collect --service-type=exec --property="SystemCallFilter=~accept" --property="SystemCallErrorNumber=ENETDOWN" ./sec_test


