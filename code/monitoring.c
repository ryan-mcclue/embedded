// SPDX-License-Identifier: zlib-acknowledgement

// or we’d catch the regression on our internal dashboards which were powered by our suite of SQL queries that would run periodically.
// 1. Application Performance Monitoring (APM, i.e. metrics): CPU usage, RAM snapshots, battery life (need to make informed business decisions)
// 2. Trace Logging: Helpful to look at when you know there is a problem (send to a database)
//   1. Plain text that is periodically flushed to circular buffer Flash
//   2. Lightweight/binary logging
//   3. Heartbeat metrics set and cleared at intervals 
// 3. Error Reporting: crashes, asserts (combine information from trace logging)  
