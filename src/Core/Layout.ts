export enum LayoutType {
    Vertical,
    Horizontal,
    Grid
}

export enum Alignment {
    Start,
    Center,
    End,
    SpaceBetween,
    SpaceAround
}

export interface Spacing {
    top: number;
    right: number;
    bottom: number;
    left: number;
}

export interface GridConfig {
    columns: number;
    rowGap: number;
    columnGap: number;
}

export class Layout {
    private type: LayoutType;
    private alignment: Alignment;
    private margin: Spacing;
    private padding: Spacing;
    private gridConfig?: GridConfig;

    constructor(type: LayoutType = LayoutType.Vertical) {
        this.type = type;
        this.alignment = Alignment.Start;
        this.margin = { top: 0, right: 0, bottom: 0, left: 0 };
        this.padding = { top: 5, right: 5, bottom: 5, left: 5 };
    }

    public setType(type: LayoutType): void {
        this.type = type;
    }

    public getType(): LayoutType {
        return this.type;
    }

    public setAlignment(alignment: Alignment): void {
        this.alignment = alignment;
    }

    public getAlignment(): Alignment {
        return this.alignment;
    }

    public setMargin(spacing: Partial<Spacing>): void {
        this.margin = { ...this.margin, ...spacing };
    }

    public getMargin(): Spacing {
        return this.margin;
    }

    public setPadding(spacing: Partial<Spacing>): void {
        this.padding = { ...this.padding, ...spacing };
    }

    public getPadding(): Spacing {
        return this.padding;
    }

    public setGridConfig(config: GridConfig): void {
        this.gridConfig = config;
        if (this.type !== LayoutType.Grid) {
            this.type = LayoutType.Grid;
        }
    }

    public getGridConfig(): GridConfig | undefined {
        return this.gridConfig;
    }

    public calculateChildPositions(
        containerSize: { width: number; height: number },
        children: Array<{ size: { width: number; height: number } }>
    ): Array<{ x: number; y: number }> {
        const positions: Array<{ x: number; y: number }> = [];
        const { padding } = this;
        const availableWidth = containerSize.width - padding.left - padding.right;
        const availableHeight = containerSize.height - padding.top - padding.bottom;

        switch (this.type) {
            case LayoutType.Vertical:
                this.calculateVerticalLayout(positions, availableWidth, availableHeight, children);
                break;
            case LayoutType.Horizontal:
                this.calculateHorizontalLayout(positions, availableWidth, availableHeight, children);
                break;
            case LayoutType.Grid:
                this.calculateGridLayout(positions, availableWidth, availableHeight, children);
                break;
        }

        // Apply padding offset to all positions
        return positions.map(pos => ({
            x: pos.x + padding.left,
            y: pos.y + padding.top
        }));
    }

    private calculateVerticalLayout(
        positions: Array<{ x: number; y: number }>,
        availableWidth: number,
        availableHeight: number,
        children: Array<{ size: { width: number; height: number } }>
    ): void {
        let currentY = 0;
        const totalHeight = children.reduce((sum, child) => sum + child.size.height, 0);
        const spacing = this.calculateSpacing(availableHeight, totalHeight, children.length);

        children.forEach((child, index) => {
            const x = this.calculateAlignedX(availableWidth, child.size.width);
            positions.push({ x, y: currentY + (index > 0 ? spacing : 0) });
            currentY += child.size.height + spacing;
        });
    }

    private calculateHorizontalLayout(
        positions: Array<{ x: number; y: number }>,
        availableWidth: number,
        availableHeight: number,
        children: Array<{ size: { width: number; height: number } }>
    ): void {
        let currentX = 0;
        const totalWidth = children.reduce((sum, child) => sum + child.size.width, 0);
        const spacing = this.calculateSpacing(availableWidth, totalWidth, children.length);

        children.forEach((child, index) => {
            const y = this.calculateAlignedY(availableHeight, child.size.height);
            positions.push({ x: currentX + (index > 0 ? spacing : 0), y });
            currentX += child.size.width + spacing;
        });
    }

    private calculateGridLayout(
        positions: Array<{ x: number; y: number }>,
        availableWidth: number,
        availableHeight: number,
        children: Array<{ size: { width: number; height: number } }>
    ): void {
        if (!this.gridConfig) return;

        const { columns, rowGap, columnGap } = this.gridConfig;
        const rows = Math.ceil(children.length / columns);
        
        children.forEach((child, index) => {
            const column = index % columns;
            const row = Math.floor(index / columns);
            
            const x = column * (availableWidth / columns) + (column * columnGap);
            const y = row * (availableHeight / rows) + (row * rowGap);
            
            positions.push({ x, y });
        });
    }

    private calculateSpacing(available: number, total: number, count: number): number {
        switch (this.alignment) {
            case Alignment.SpaceBetween:
                return count > 1 ? (available - total) / (count - 1) : 0;
            case Alignment.SpaceAround:
                return count > 0 ? (available - total) / (count * 2) : 0;
            default:
                return 0;
        }
    }

    private calculateAlignedX(containerWidth: number, childWidth: number): number {
        switch (this.alignment) {
            case Alignment.Center:
                return (containerWidth - childWidth) / 2;
            case Alignment.End:
                return containerWidth - childWidth;
            default:
                return 0;
        }
    }

    private calculateAlignedY(containerHeight: number, childHeight: number): number {
        switch (this.alignment) {
            case Alignment.Center:
                return (containerHeight - childHeight) / 2;
            case Alignment.End:
                return containerHeight - childHeight;
            default:
                return 0;
        }
    }
}
