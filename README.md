A tiny fast graph database engine written in C

Note: in very early-stage development

Design goals:
- Easy to embed
- ACID transactions with snapshot isolation
- Fast lock-free reads and traversals (with single writer)
- Supernode optimization
- Reduces to a key/value store if nodes have no edges
