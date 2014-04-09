A tiny fast graph database engine written in C

Note: this is in very early-stage development

Design goals:
- Clean modular code
- Easy to embed
- ACID transactions with snapshot isolation
- Fast, lock-free reads and traversals (with single writer)
- Supernode optimization
- Reduces to a key/value store if nodes have no edges
