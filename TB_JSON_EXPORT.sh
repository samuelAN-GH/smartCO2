#!/bin/bash

JWT_TOKEN="eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJ0ZW5hbnRAdGhpbmdzYm9hcmQub3JnIiwic2NvcGVzIjpbIlRFTkFOVF9BRE1JTiJdLCJ1c2VySWQiOiJlYzliOWZiMC04MWFkLTExZWMtYTc2Ni1iNTU0MjYwYzVlMWUiLCJlbmFibGVkIjp0cnVlLCJpc1B1YmxpYyI6ZmFsc2UsInRlbmFudElkIjoiZWFlMzY4NjAtODFhZC0xMWVjLWE3NjYtYjU1NDI2MGM1ZTFlIiwiY3VzdG9tZXJJZCI6IjEzODE0MDAwLTFkZDItMTFiMi04MDgwLTgwODA4MDgwODA4MCIsImlzcyI6InRoaW5nc2JvYXJkLmlvIiwiaWF0IjoxNjQ0MTg0ODc3LCJleHAiOjE2NzUyODg4Nzd9.847JuRoa2XFbBCm-Q8BUkbm99unPToyftDaY2W8IgCg3t7uptFpse8zP8NTdq0DoSYI2ax8m3GrI6A4skG5Kpg"
THINGSBOARD_URL="http://192.168.0.20:8080"
DEVICEID="b3cea640-81ae-11ec-bc4c-f979b2fe7c97"
KEYS="co2,rh,temp"
ENDTS="1644218332000"
OUTFILE="tb-data.json"

curl -X GET \
--header "Content-Type:application/json" \
--header "X-Authorization: Bearer ${JWT_TOKEN}" \
"${THINGSBOARD_URL}/api/plugins/telemetry/DEVICE/${DEVICEID}/values/timeseries?keys=${KEYS}&startTs=0000000000000&endTs=${ENDTS}&agg=NONE&limit=100000000" \
-o $OUTFILE