import { Vec3 } from './Math';

/**
 * Quaternion class for representing rotations.
 * Uses (x, y, z, w) format where w is the scalar component.
 */
export class Quaternion {
    constructor(
        public x: number = 0,
        public y: number = 0,
        public z: number = 0,
        public w: number = 1
    ) {}

    /**
     * Creates a quaternion from axis-angle representation
     */
    static fromAxisAngle(axis: Vec3, angle: number): Quaternion {
        const halfAngle = angle * 0.5;
        const s = Math.sin(halfAngle);
        return new Quaternion(
            axis.x * s,
            axis.y * s,
            axis.z * s,
            Math.cos(halfAngle)
        );
    }

    /**
     * Creates a quaternion from Euler angles (in radians)
     */
    static fromEuler(x: number, y: number, z: number): Quaternion {
        const cx = Math.cos(x * 0.5);
        const cy = Math.cos(y * 0.5);
        const cz = Math.cos(z * 0.5);
        const sx = Math.sin(x * 0.5);
        const sy = Math.sin(y * 0.5);
        const sz = Math.sin(z * 0.5);

        return new Quaternion(
            sx * cy * cz + cx * sy * sz,
            cx * sy * cz - sx * cy * sz,
            cx * cy * sz + sx * sy * cz,
            cx * cy * cz - sx * sy * sz
        );
    }

    /**
     * Returns the length/magnitude of the quaternion
     */
    length(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w);
    }

    /**
     * Normalizes the quaternion
     */
    normalize(): Quaternion {
        const len = this.length();
        if (len === 0) return this;
        
        const invLen = 1 / len;
        this.x *= invLen;
        this.y *= invLen;
        this.z *= invLen;
        this.w *= invLen;
        return this;
    }

    /**
     * Returns a normalized copy of the quaternion
     */
    normalized(): Quaternion {
        return new Quaternion(this.x, this.y, this.z, this.w).normalize();
    }

    /**
     * Multiplies this quaternion by another (combines rotations)
     */
    multiply(q: Quaternion): Quaternion {
        const x = this.x * q.w + this.w * q.x + this.y * q.z - this.z * q.y;
        const y = this.y * q.w + this.w * q.y + this.z * q.x - this.x * q.z;
        const z = this.z * q.w + this.w * q.z + this.x * q.y - this.y * q.x;
        const w = this.w * q.w - this.x * q.x - this.y * q.y - this.z * q.z;
        
        return new Quaternion(x, y, z, w);
    }

    /**
     * Returns the conjugate of this quaternion
     */
    conjugate(): Quaternion {
        return new Quaternion(-this.x, -this.y, -this.z, this.w);
    }

    /**
     * Returns the inverse of this quaternion
     */
    inverse(): Quaternion {
        const len = this.length();
        if (len === 0) return new Quaternion();
        
        const invLen = 1 / (len * len);
        return new Quaternion(
            -this.x * invLen,
            -this.y * invLen,
            -this.z * invLen,
            this.w * invLen
        );
    }

    /**
     * Rotates a vector by this quaternion
     */
    rotateVector(v: Vec3): Vec3 {
        const u = new Vec3(this.x, this.y, this.z);
        const s = this.w;
        
        const dot2 = 2.0 * (u.x * v.x + u.y * v.y + u.z * v.z);
        const cross = new Vec3(
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        );

        return new Vec3(
            v.x + 2.0 * (s * cross.x + u.x * dot2),
            v.y + 2.0 * (s * cross.y + u.y * dot2),
            v.z + 2.0 * (s * cross.z + u.z * dot2)
        );
    }

    /**
     * Converts the quaternion to Euler angles (in radians)
     */
    toEuler(): { x: number; y: number; z: number } {
        const x = Math.atan2(2 * (this.w * this.x + this.y * this.z), 1 - 2 * (this.x * this.x + this.y * this.y));
        const y = Math.asin(2 * (this.w * this.y - this.z * this.x));
        const z = Math.atan2(2 * (this.w * this.z + this.x * this.y), 1 - 2 * (this.y * this.y + this.z * this.z));
        
        return { x, y, z };
    }

    /**
     * Linear interpolation between two quaternions
     */
    static lerp(a: Quaternion, b: Quaternion, t: number): Quaternion {
        const t1 = 1 - t;
        return new Quaternion(
            t1 * a.x + t * b.x,
            t1 * a.y + t * b.y,
            t1 * a.z + t * b.z,
            t1 * a.w + t * b.w
        ).normalize();
    }

    /**
     * Spherical linear interpolation between two quaternions
     */
    static slerp(a: Quaternion, b: Quaternion, t: number): Quaternion {
        let cosHalfTheta = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        
        if (Math.abs(cosHalfTheta) >= 1.0) {
            return a;
        }

        if (cosHalfTheta < 0) {
            b = new Quaternion(-b.x, -b.y, -b.z, -b.w);
            cosHalfTheta = -cosHalfTheta;
        }

        const halfTheta = Math.acos(cosHalfTheta);
        const sinHalfTheta = Math.sqrt(1.0 - cosHalfTheta * cosHalfTheta);

        if (Math.abs(sinHalfTheta) < 0.001) {
            return new Quaternion(
                a.x * 0.5 + b.x * 0.5,
                a.y * 0.5 + b.y * 0.5,
                a.z * 0.5 + b.z * 0.5,
                a.w * 0.5 + b.w * 0.5
            );
        }

        const ratioA = Math.sin((1 - t) * halfTheta) / sinHalfTheta;
        const ratioB = Math.sin(t * halfTheta) / sinHalfTheta;

        return new Quaternion(
            a.x * ratioA + b.x * ratioB,
            a.y * ratioA + b.y * ratioB,
            a.z * ratioA + b.z * ratioB,
            a.w * ratioA + b.w * ratioB
        );
    }

    /**
     * Returns a string representation of the quaternion
     */
    toString(): string {
        return `Quaternion(${this.x}, ${this.y}, ${this.z}, ${this.w})`;
    }
}
